
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>

#include <spii/auto_diff_term.h>
#include <spii/solver.h>
using namespace spii;

#include "hastighet.h"

// Read a Bundle Adjustment in the Large dataset.
class BALProblem {
public:

	BALProblem(const std::string& filename)
	{
		auto result = LoadFile(filename.c_str());
		spii_assert(result, "Could not load BA problem file.");
	}

	int number_of_observations()       const { return num_observations;                  }
	int number_of_cameras()            const { return num_cameras;                       }
	int number_of_points()             const { return num_points;                        }
	const double* get_observations()   const { return &observations[0];                   }
	double* mutable_cameras()                { return &parameters[0];                    }
	double* mutable_points()                 { return &parameters[0]  + 9 * num_cameras; }
	void reset_parameters() { parameters = original_parameters; }

	double* mutable_camera_for_observation(int i)
	{
		return mutable_cameras() + camera_index[i] * 9;
	}

	double* mutable_point_for_observation(int i)
	{
		return mutable_points() + point_index[i] * 3;
	}

	bool LoadFile(const char* filename) {
		std::ifstream fin(filename);
		if (!fin) {
			return false;
		}

		fin >> num_cameras >> num_points >>num_observations;

		if (!fin) {
			return false;
		}

		point_index.resize(num_observations);
		camera_index.resize(num_observations);
		observations.resize(2 * num_observations);

		num_parameters = 9 * num_cameras + 3 * num_points;
		parameters.resize(num_parameters);

		for (int i = 0; i < num_observations; ++i) {
			fin >> camera_index[i];
			fin >> point_index[i];
			for (int j = 0; j < 2; ++j) {
				fin >> observations[2*i + j];
			}
			if (!fin) {
				return false;
			}
		}

		for (int i = 0; i < num_parameters; ++i) {
			fin >> parameters[i];
		}

		if (!fin) {
			return false;
		}

		original_parameters = parameters;

		return true;
	}

private:
	int num_cameras;
	int num_points;
	int num_observations;
	int num_parameters;

	std::vector<int> point_index;
	std::vector<int> camera_index;
	std::vector<double> observations;
	std::vector<double> parameters;
	std::vector<double> original_parameters;
};

template<typename T> inline
T dot_product(const T x[3], const T y[3]) {
	return (x[0] * y[0] + x[1] * y[1] + x[2] * y[2]);
}

template<typename T> inline
void cross_product(const T x[3], const T y[3], T x_cross_y[3]) {
	x_cross_y[0] = x[1] * y[2] - x[2] * y[1];
	x_cross_y[1] = x[2] * y[0] - x[0] * y[2];
	x_cross_y[2] = x[0] * y[1] - x[1] * y[0];
}

//
// Function from Ceres Solver.
//
template<typename T> inline
void angle_axis_rotate_point(const T angle_axis[3], const T pt[3], T result[3]) {
	T w[3];
	T sintheta;
	T costheta;

	const T theta2 = dot_product(angle_axis, angle_axis);
	if (theta2 > 0.0) {
		// Away from zero, use the rodriguez formula
		//
		//   result = pt costheta +
		//            (w x pt) * sintheta +
		//            w (w . pt) (1 - costheta)
		//
		// We want to be careful to only evaluate the square root if the
		// norm of the angle_axis vector is greater than zero. Otherwise
		// we get a division by zero.
		//
		const T theta = sqrt(theta2);
		w[0] = angle_axis[0] / theta;
		w[1] = angle_axis[1] / theta;
		w[2] = angle_axis[2] / theta;
		costheta = cos(theta);
		sintheta = sin(theta);
		T w_cross_pt[3];
		cross_product(w, pt, w_cross_pt);
		T w_dot_pt = dot_product(w, pt);
		for (int i = 0; i < 3; ++i) {
			result[i] = pt[i] * costheta +
			w_cross_pt[i] * sintheta +
			w[i] * (T(1.0) - costheta) * w_dot_pt;
		}
	} else {
		// Near zero, the first order Taylor approximation of the rotation
		// matrix R corresponding to a vector w and angle w is
		//
		//   R = I + hat(w) * sin(theta)
		//
		// But sintheta ~ theta and theta * w = angle_axis, which gives us
		//
		//  R = I + hat(w)
		//
		// and actually performing multiplication with the point pt, gives us
		// R * pt = pt + w x pt.
		//
		// Switching to the Taylor expansion at zero helps avoid all sorts
		// of numerical nastiness.
		T w_cross_pt[3];
		cross_product(angle_axis, pt, w_cross_pt);
		for (int i = 0; i < 3; ++i) {
			result[i] = pt[i] + w_cross_pt[i];
		}
	}
}

//
// Code from Ceres Solver.
//
// Templated pinhole camera model for used with Ceres.  The camera is
// parameterized using 9 parameters: 3 for rotation, 3 for translation, 1 for
// focal length and 2 for radial distortion. The principal point is not modeled
// (i.e. it is assumed be located at the image center).
class SnavelyReprojectionError {
public:
	SnavelyReprojectionError(double observed_x, double observed_y)
	: observed_x(observed_x), observed_y(observed_y) {}

	template <typename T>
	T operator()(const T* const camera,
	             const T* const point) const {
		// camera[0,1,2] are the angle-axis rotation.
		T p[3];
		angle_axis_rotate_point(camera, point, p);

		// camera[3,4,5] are the translation.
		p[0] += camera[3];
		p[1] += camera[4];
		p[2] += camera[5];

		// Compute the center of distortion. The sign change comes from
		// the camera model that Noah Snavely's Bundler assumes, whereby
		// the camera coordinate system has a negative z axis.
		T xp = - p[0] / p[2];
		T yp = - p[1] / p[2];

		// Apply second and fourth order radial distortion.
		const T& l1 = camera[7];
		const T& l2 = camera[8];
		T r2 = xp*xp + yp*yp;
		T distortion = T(1.0) + r2  * (l1 + l2  * r2);

		// Compute final projected point position.
		const T& focal = camera[6];
		T predicted_x = focal * distortion * xp;
		T predicted_y = focal * distortion * yp;

		// The error is the difference between the predicted and observed position.
		T r0 = predicted_x - T(observed_x);
		T r1 = predicted_y - T(observed_y);

		return 0.5 * (r0*r0 + r1*r1);
	}

private:
	double observed_x;
	double observed_y;
};


template<typename SolverClass>
class BundleAdjustmentBenchmark :
	public hastighet::Test
{
public:
	Function function;
	BALProblem bal_problem;
	SolverClass solver;
	SolverResults results;		

	BundleAdjustmentBenchmark() :
		bal_problem("balproblem.txt")
	{
		try {
			// Create residuals for each observation in the bundle adjustment problem. The
			// parameters for cameras and points are added automatically.
			
			for (int i = 0; i < bal_problem.number_of_observations(); ++i) {
				// Add camera
				function.add_variable(bal_problem.mutable_camera_for_observation(i), 9);
				// Add point
				function.add_variable(bal_problem.mutable_point_for_observation(i), 3);
				// Each Residual block takes a point and a camera as input and outputs a 2
				// dimensional residual. Internally, the cost function stores the observed
				// image location and compares the reprojection against the observation.
				auto term =
					std::make_shared<AutoDiffTerm<SnavelyReprojectionError, 9, 3>>(
							bal_problem.get_observations()[2 * i + 0],
							bal_problem.get_observations()[2 * i + 1]);

				function.add_term(term,
					bal_problem.mutable_camera_for_observation(i),
					bal_problem.mutable_point_for_observation(i));
			}

			solver.function_improvement_tolerance = 1e-5;
			solver.log_function = [](const std::string&) { };
		}
		catch (std::exception& e) {
			std::cerr << e.what() << '\n';
			throw;
		}
	}
};

typedef BundleAdjustmentBenchmark<NewtonSolver> BundleAdjustmentBenchmarkNewtonSolver;
BENCHMARK_F(BundleAdjustmentBenchmarkNewtonSolver, one_newton_iteration)
{
	bal_problem.reset_parameters();
	solver.sparsity_mode = NewtonSolver::SparsityMode::SPARSE;
	solver.maximum_iterations = 1;
	solver.solve(function, &results);
}

typedef BundleAdjustmentBenchmark<LBFGSSolver> BundleAdjustmentBenchmarkLBFGSSolver;
BENCHMARK_F(BundleAdjustmentBenchmarkLBFGSSolver, ten_lbfgs_iterations)
{
	bal_problem.reset_parameters();
	solver.maximum_iterations = 10;
	solver.solve(function, &results);
}

int main(int argc, char** argv)
{
	try
	{
		hastighet::Benchmarker::RunAllTests(argc, argv);
	}
	catch (std::bad_alloc&)
	{
		std::cerr << "\nOut of memory. Exiting.\n";
		return 1;
	}
}

// Petter Strandmark 2012.

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include <Eigen/Dense>

#include <spii/spii.h>
#include <spii/solver.h>

namespace spii {

// Holds a point in the Nelder-Mead simplex.
// Equipped with a comparison operator for sorting.
struct SimplexPoint
{
	Eigen::VectorXd x;
	double value;

	bool operator<(const SimplexPoint& rhs) const
	{
		return this->value < rhs.value;
	}
};

// If required for debugging.
std::ostream& operator<<(std::ostream& out, const SimplexPoint& point)
{
	out << point.x.transpose() << " : " << point.value;
	return out;
}

}  // namespace spii

namespace std
{
	template<>
	void swap<spii::SimplexPoint>(spii::SimplexPoint& lhs, spii::SimplexPoint& rhs)
	{
		lhs.x.swap(rhs.x);
		swap(lhs.value, rhs.value);
	}
}

namespace spii {

void initialize_simplex(const Function& function,
                        const Eigen::VectorXd& x0,
                        std::vector<SimplexPoint>* simplex)
{
	size_t n = function.get_number_of_scalars();
	Eigen::VectorXd absx0 = x0;
	for (size_t i = 0; i < n; ++i) {
		absx0[i] = std::abs(x0[i]);
	}
	double scale = std::max(absx0.maxCoeff(), 1.0);
	const double nd = static_cast<double>(n);
	double alpha1 = scale / (nd * std::sqrt(2.0)) * (std::sqrt(nd+1)- 1 + nd);
	double alpha2 = scale / (nd * std::sqrt(2.0)) * (std::sqrt(nd+1) - 1);
	Eigen::VectorXd alpha2_vec(x0.size());
	alpha2_vec.setConstant(alpha2);

	simplex->at(0).x = x0;
	for (size_t i = 1; i < n + 1; ++i) {
		simplex->at(i).x  = x0 + alpha2_vec;
		simplex->at(i).x[i-1] = x0[i-1] + alpha1;
	}

	for (size_t i = 0; i < n + 1; ++i) {
		simplex->at(i).value = function.evaluate(simplex->at(i).x);
	}

	std::sort(simplex->begin(), simplex->end());
}

void NelderMeadSolver::solve(const Function& function,
                             SolverResults* results) const
{
	double global_start_time = wall_time();

	// Dimension of problem.
	size_t n = function.get_number_of_scalars();

	if (n == 0) {
		results->exit_condition = SolverResults::FUNCTION_TOLERANCE;
		return;
	}

	// The Nelder-Mead simplex.
	std::vector<SimplexPoint> simplex(n + 1);

	// Copy the user state to the current point.
	Eigen::VectorXd x;
	function.copy_user_to_global(&x);

	initialize_simplex(function, x, &simplex);

	SimplexPoint mean_point;
	SimplexPoint reflection_point;
	SimplexPoint expansion_point;
	mean_point.x.resize(n);
	reflection_point.x.resize(n);
	expansion_point.x.resize(n);

	double fmin  = std::numeric_limits<double>::quiet_NaN();
	double fmax  = std::numeric_limits<double>::quiet_NaN();
	double fval  = std::numeric_limits<double>::quiet_NaN();
	double area  = std::numeric_limits<double>::quiet_NaN();
	double area0 = std::numeric_limits<double>::quiet_NaN();
	double length  = std::numeric_limits<double>::quiet_NaN();
	double length0 = std::numeric_limits<double>::quiet_NaN();

	Eigen::MatrixXd area_mat(n, n);

	//
	// START MAIN ITERATION
	//
	results->startup_time   += wall_time() - global_start_time;
	results->exit_condition = SolverResults::INTERNAL_ERROR;
	int iter = 0;
	int n_shrink_in_a_row = 0;
	while (true) {

		//
		// In each iteration, the worst point in the simplex
		// is replaced with a new one.
		//
		double start_time = wall_time();

		mean_point.x.setZero();
		fval = 0;
		// Compute the mean of the best n points.
		for (size_t i = 0; i < n; ++i) {
			mean_point.x += simplex[i].x;
			fval         += simplex[i].value;
		}
		fval         /= double(n);
		mean_point.x /= double(n);
		fmin = simplex[0].value;
		fmax = simplex[n].value;

		const char* iteration_type = "n/a";

		// Compute the reflexion point and evaluate it.
		reflection_point.x = 2.0 * mean_point.x - simplex[n].x;
		reflection_point.value = function.evaluate(reflection_point.x);

		bool is_shrink = false;
		if (simplex[0].value <= reflection_point.value &&
			reflection_point.value < simplex[n - 1].value) {
			// Reflected point is neither better nor worst in the
			// new simplex.
			std::swap(reflection_point, simplex[n]);
			iteration_type = "Reflect 1";
		}
		else if (reflection_point.value < simplex[0].value) {
			// Reflected point is better than the current best; try
			// to go farther along this direction.

			// Compute expansion point.
			expansion_point.x = 3.0 * mean_point.x - 2.0 * simplex[n].x;
			expansion_point.value = function.evaluate(expansion_point.x);

			if (expansion_point.value < reflection_point.value) {
				std::swap(expansion_point, simplex[n]);
				iteration_type = "Expansion";
			}
			else {
				std::swap(reflection_point, simplex[n]);
				iteration_type = "Reflect 2";
			}
		}
		else {
			// Reflected point is still worse than x[n]; contract.
			bool success = false;

			if (simplex[n - 1].value <= reflection_point.value &&
			    reflection_point.value < simplex[n].value) {
				// Try to perform "outside" contraction.
				expansion_point.x = 1.5 * mean_point.x - 0.5 * simplex[n].x;
				expansion_point.value = function.evaluate(expansion_point.x);

				if (expansion_point.value <= reflection_point.value) {
					std::swap(expansion_point, simplex[n]);
					success = true;
					iteration_type = "Outside contraction";
				}
			}
			else {
				// Try to perform "inside" contraction.
				expansion_point.x = 0.5 * mean_point.x + 0.5 * simplex[n].x;
				expansion_point.value = function.evaluate(expansion_point.x);

				if (expansion_point.value < simplex[n].value) {
					std::swap(expansion_point, simplex[n]);
					success = true;
					iteration_type = "Inside contraction";
				}
			}

			if (! success) {
				// Neither outside nor inside contraction was acceptable;
				// shrink the simplex toward the best point.
				for (size_t i = 1; i < n + 1; ++i) {
					simplex[i].x = 0.5 * (simplex[0].x + simplex[i].x);
					simplex[i].value = function.evaluate(simplex[i].x);
					iteration_type = "Shrink";
					is_shrink = true;
				}
			}
		}

		std::sort(simplex.begin(), simplex.end());

		results->function_evaluation_time += wall_time() - start_time;

		//
		// Test stopping criteriea
		//
		start_time = wall_time();
		
		// Compute the area of the simplex.
		length = 0;
		for (size_t i = 0; i < n; ++i) {
			area_mat.col(i) = simplex[i].x - simplex[n].x;
			length = std::max(length, area_mat.col(i).norm());
		}
		area = std::abs(area_mat.determinant());
		if (iter == 0) {
			area0 = area;
			length0 = length;
		}

		if (area / area0 < this->area_tolerance) {
			results->exit_condition = SolverResults::GRADIENT_TOLERANCE;
			break;
		}

		if (area == 0) {
			results->exit_condition = SolverResults::GRADIENT_TOLERANCE;
			break;
		}

		if (length / length0 < this->length_tolerance) {
			results->exit_condition = SolverResults::GRADIENT_TOLERANCE;
			break;
		}

		if (is_shrink) {
			n_shrink_in_a_row++;
		}
		else {
			n_shrink_in_a_row = 0;
		}
		if (n_shrink_in_a_row > 50) {
			results->exit_condition = SolverResults::GRADIENT_TOLERANCE;
			break;
		}

		if (iter >= this->maximum_iterations) {
			results->exit_condition = SolverResults::NO_CONVERGENCE;
			break;
		}

		if (this->callback_function) {
			CallbackInformation information;
			information.objective_value = simplex[0].value;
			information.x = &simplex[0].x;

			if (!callback_function(information)) {
				results->exit_condition = SolverResults::USER_ABORT;
				break;
			}
		}
		results->stopping_criteria_time += wall_time() - start_time;

		//
		// Restarting
		//
		//if (area / area1 < 1e-10) {
		//	x = simplex[0].x;
		//	initialize_simplex(function, x, &simplex);
		//	area1 = area;
		//	if (this->log_function) {
		//		this->log_function("Restarted.");
		//	}
		//}

		//
		// Log the results of this iteration.
		//
		start_time = wall_time();

		int log_interval = 1;
		if (iter > 30) {
			log_interval = 10;
		}
		if (iter > 200) {
			log_interval = 100;
		}
		if (iter > 2000) {
			log_interval = 1000;
		}
		if (this->log_function && iter % log_interval == 0) {
			char str[1024];
				if (iter == 0) {
					this->log_function("Itr     min(f)     avg(f)     max(f)    area    length   type");
				}
				std::sprintf(str, "%6d %+.3e %+.3e %+.3e %.3e %.3e %s",
					iter, fmin, fval, fmax, area, length, iteration_type);
			this->log_function(str);
		}
		results->log_time += wall_time() - start_time;

		iter++;
	}

	// Return the best point as solution.
	function.copy_global_to_user(simplex[0].x);
	results->total_time += wall_time() - global_start_time;

	if (this->log_function) {
		char str[1024];
		std::sprintf(str, " end   %+.3e                       %.3e %.3e", fval, area, length);
		this->log_function(str);
	}
}

}  // namespace spii

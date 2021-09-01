// Petter Strandmark 2013.
//
// Run as “curvature > output.data”
//
// The data can then be visualized with e.g. the following
// Python script.
//
//		import numpy as np
//		import matplotlib.pyplot as plt
//
//		A = np.loadtxt('curvature.txt')
//
//		plt.hold(True)
//		plt.plot(A[0, :], A[1, :], '.-k')
//		plt.plot(A[2, :], A[3, :], '.-b')
//		plt.plot(A[4, :], A[5, :], '.-r')
//		plt.legend(['Start', '"Length"', 'Curvature'])
//
//
// See also [1], [2].
//
//  [1] Petter Strandmark, Johannes Ulén, Fredrik Kahl, Leo Grady.
//      Shortest Paths with Curvature and Torsion. International
//      Conference on Computer Vision. 2013.
//
//  [2] https://github.com/PetterS/curve_extraction .
//

#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>

#include <spii/auto_diff_term.h>
#include <spii/solver.h>

using namespace spii;

struct Point
{
	double xy[2];
};

struct Distance
{
	template<typename R>
	R operator()(const R* const point1, const R* const point2) const
	{
		using std::sqrt;
		R dx = point1[0] - point2[0];
		R dy = point1[1] - point2[1];
		return (dx*dx + dy*dy);
	}
};

std::ostream& operator << (std::ostream& out, const std::vector<Point>& points)
{
	for (int i = 0; i < points.size(); ++i) {
		out << points[i].xy[0] << " ";
	}
	out << std::endl;
	for (int i = 0; i < points.size(); ++i) {
		out << points[i].xy[1] << " ";
	}
	out << std::endl;
	return out;
}

//
// Computes the integral of the curvature to the power of p
// along the spline defined by the three input points.
//
template<typename R>
R compute_curvature(R x1, R y1, R z1,
                    R x2, R y2, R z2,
                    R x3, R y3, R z3,
                    double p, int n)
{
	using std::pow;

	const R a = x1 - 2*x2 + x3;
	const R b = y1 - 2*y2 + y3;
	const R c = z1 - 2*z2 + z3;
	const R dx = x1 - x2;
	const R dy = y1 - y2;
	const R dz = z1 - z2;

	const R sq1 = c * dy - b * dz;
	const R sq2 = c * dx - a * dz;
	const R sq3 = b * dx - a * dy;

	const R numerator = pow(sq1*sq1 + sq2*sq2 + sq3*sq3, p / R(2.0));

	// k(t)^p * ds
	auto kp_ds = [a,b,c,dx,dy,dz,numerator,p](R t) -> R
	{
		const R sq4 = a * t - dx;
		const R sq5 = b * t - dy;
		const R sq6 = c * t - dz;
		const double exponent = 3 * p / 2.0 - 0.5;
		return numerator / pow(sq4*sq4 + sq5*sq5 + sq6*sq6, exponent);
	};

	// Trapezoidal rule.
	R sum = 0;
	sum += kp_ds(0) / R(2.0);
	sum += kp_ds(1) / R(2.0);
	for (int k = 1; k <= n - 1; ++k) {
		sum += kp_ds(R(k) / R(n));
	}
	sum /= R(n);

	return sum;
}

struct Curvature
{
	template<typename R>
	R operator()(const R* const point1, const R* const point2, const R* const point3) const
	{
		return 100 * compute_curvature<R>(point1[0], point1[1], 0,
		                                  point2[0], point2[1], 0,
		                                  point3[0], point3[1], 0,
		                                  2.0, 100);
	}
};

int main_function()
{
	using namespace std;

	Function f;
	int n = 20;

	// Create the points and add them as variables
	// to the function.
	vector<Point> points(n);
	for (int i = 0; i < n/2; ++i) {
		points[i].xy[0] = i;
		points[i].xy[1] = 0;
		f.add_variable(points[i].xy, 2);
	}
	for (int i = n/2; i < n; ++i) {
		points[i].xy[0] = n/2;
		points[i].xy[1] = i - n/2;
		f.add_variable(points[i].xy, 2);
	}

	cout << points;

	// Add terms measuring the distance between two points.
	auto distance = make_shared<AutoDiffTerm<Distance, 2, 2>>();
	for (int i = 1; i < n; ++i) {
		f.add_term(distance, points[i-1].xy, points[i].xy);
	}

	// The start and end of the curve is fixed.
	f.set_constant(points[0].xy, true);
	f.set_constant(points[1].xy, true);
	f.set_constant(points[n-2].xy, true);
	f.set_constant(points[n-1].xy, true);

	NewtonSolver solver;
	SolverResults results;
	solver.solve(f, &results);
	cerr << results << endl;

	cout << points;

	// Add some curvature.
	auto curvature = make_shared<AutoDiffTerm<Curvature, 2, 2, 2>>();
	for (int i = 2; i < n; ++i) {
		vector<double*> args;
		args.push_back(points[i-2].xy);
		args.push_back(points[i-1].xy);
		args.push_back(points[i].xy);

		f.add_term(curvature, args);
	}

	// Slight pertubation.
	mt19937_64 engine;
	auto rand = uniform_real_distribution<double>(0, 1e-3);
	for (int i = 2; i < n-2; ++i) {
		points[i].xy[0] += rand(engine);
		points[i].xy[1] += rand(engine);
	}

	solver.factorization_method = NewtonSolver::FactorizationMethod::ITERATIVE;
	solver.maximum_iterations = 1000;
	solver.solve(f, &results);
	cerr << results << endl;

	cout << points;

	return 0;
}

int main()
{
	try {
		return main_function();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}

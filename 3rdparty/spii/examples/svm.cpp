// Petter Strandmark 2013.
//
// Example of constrained optimization. Finds an optimal
// separating hyperplane between two sets of points in 2D.
//
// Note that this is a convex quadratic program. There are
// more efficient methods to solve this problem than
// presented in this example.

#include <iostream>
#include <stdexcept>

#include <spii/auto_diff_term.h>
#include <spii/constrained_function.h>
#include <spii/solver.h>

// ||w||^2
class Norm
{
public:
	template<typename R>
	R operator()(const R* const w) const
	{
		return w[0]*w[0] + w[1]*w[1];
	}
};

// yi * (w^T * xi + b) ≤ -1.0.
class SVMConstraint
{
public:
	SVMConstraint(int y_, const Eigen::Vector2d& x_)
		: y{y_}, x{x_} 
	{ }

	template<typename R>
	R operator()(const R* const w, const R* const b) const
	{
		return R(y) * (w[0]*x(0) + w[1]*x[1] + *b) + 1.0;
	}
private:
	const int y;
	const Eigen::Vector2d x;
};

int main_function()
{
	using namespace spii;
	using namespace std;

	ConstrainedFunction function;
	
	// 2D coordinates of every point.
	vector<Eigen::Vector2d> x =
	{
		{0.0243,   -0.1624},
		{-0.6057,    0.2717},
		{0.3586,   -0.3523},
		{0.1324,    2.6736},
		{-0.0076,    0.7103},
		{-0.4625,   -0.5279},
		{-0.3458,    0.5646},
		{-0.8015,   -1.8529},
		{0.0532,  -0.4364},
		{0.2800,    0.0846},
		{0.7291,    0.9008},
		{-1.3256,   -1.5246},
		{0.5024,   0.3064},
		{-0.9392,    1.2374},
		{0.0330,    0.1236},
		{0.2739,    0.7542},
		{1.5245,   -0.2620},
		{-1.7307,    1.0135},
		{-0.9277,    1.2437},
		{-0.5895,    1.4915},
		{-0.8060,   -1.1126},
		{0.9569,  -1.3631},
		{0.6806,    0.4378},
		{-0.1433,   -0.3415},
		{2.0578,   -0.3745},
		{-0.1902,   -0.9387},
		{-0.8135,   -0.5836},
		{1.7095,   -0.3904},
		{-0.6901,   -0.4482},
		{0.7144,   -1.3157}
	};

	// Class of every point.
	vector<int> y = {-1, -1, 1, 1,  1, -1, -1, -1, -1,  1,  1, -1,  1, -1,  1,  1,  1, 
	                 -1, -1,  1, -1,  1,  1, -1,  1, -1, -1,  1, -1,  1};

	spii_assert(y.size() == x.size());

	Eigen::Vector2d w = {0, 0};
	double b = 0;

	function.add_term<AutoDiffTerm<Norm, 2>>(&w(0));

	for (size_t i = 0; i < y.size(); ++i) {
		function.add_constraint_term(
			to_string(i), 
			make_shared<AutoDiffTerm<SVMConstraint, 2, 1>>(y[i], x[i]),
			&w[0], 
			&b);
	}

	LBFGSSolver solver;
	SolverResults results;
	function.solve(solver, &results);

	cout << results << endl << endl;
	cout << "||w|| = " << w.norm() << endl;
	cout << "w = (" << w[0] << ", " << w[1] << ")\n";
	cout << "b = " << b << endl;

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

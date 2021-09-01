// Petter Strandmark 2012-2013.

#include <cmath>
#include <limits>
#include <random>

#include <catch.hpp>
#include <spii/google_test_compatibility.h>

#include <spii/auto_diff_term.h>
#include <spii/solver.h>
#include <spii/transformations.h>

using namespace spii;

struct Rosenbrock
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 =  x[1] - x[0]*x[0];
		R d1 =  1 - x[0];
		return 100 * d0*d0 + d1*d1;
	}
};

void test_method(const Solver& solver)
{
	Function f;
	double x[2] = {-1.2, 1.0};
	f.add_term(std::make_shared<AutoDiffTerm<Rosenbrock, 2>>(), x);

	SolverResults results;
	solver.solve(f, &results);

	EXPECT_TRUE(results.exit_condition == SolverResults::ARGUMENT_TOLERANCE ||
	            results.exit_condition == SolverResults::FUNCTION_TOLERANCE ||
	            results.exit_condition == SolverResults::GRADIENT_TOLERANCE);
	EXPECT_LT( std::fabs(x[0] - 1.0), 1e-9);
	EXPECT_LT( std::fabs(x[1] - 1.0), 1e-9);
	EXPECT_LT( std::fabs(f.evaluate()), 1e-9);
}

TEST(Solver, NEWTON)
{
	NewtonSolver solver;
	solver.log_function = nullptr;
	test_method(solver);
}

TEST(Solver, LBFGS)
{
	LBFGSSolver solver;
	solver.log_function = nullptr;
	test_method(solver);
}

TEST(Solver, NELDER_MEAD)
{
	NelderMeadSolver solver;
	solver.log_function = nullptr;
	solver.maximum_iterations = 10000;
	solver.area_tolerance = 1e-40;
	test_method(solver);
}

TEST(Solver, PATTERN_SEARCH)
{
	PatternSolver solver;
	solver.log_function = nullptr;
	solver.maximum_iterations = 100000;
	test_method(solver);
}

TEST(Solver, function_tolerance)
{
	Function f;
	double x[2] = {-1.2, 1.0};
	f.add_variable(x, 2);
	f.add_term(std::make_shared<AutoDiffTerm<Rosenbrock, 2>>(), x);

	NewtonSolver solver;
	solver.log_function = nullptr;
	solver.maximum_iterations = 50;
	solver.gradient_tolerance = 0;
	solver.argument_improvement_tolerance = 0;
	SolverResults results;
	solver.solve(f, &results);

	EXPECT_TRUE(results.exit_condition == SolverResults::FUNCTION_TOLERANCE);
}

TEST(Solver, argument_improvement_tolerance)
{
	Function f;
	double x[2] = {-1.2, 1.0};
	f.add_variable(x, 2);
	f.add_term(std::make_shared<AutoDiffTerm<Rosenbrock, 2>>(), x);

	NewtonSolver solver;
	solver.log_function = nullptr;
	solver.maximum_iterations = 50;
	solver.gradient_tolerance = 0;
	solver.function_improvement_tolerance = 0;
	SolverResults results;
	solver.solve(f, &results);

	EXPECT_TRUE(results.exit_condition == SolverResults::ARGUMENT_TOLERANCE);
}

TEST(Solver, gradient_tolerance)
{
	Function f;
	double x[2] = {-1.2, 1.0};
	f.add_variable(x, 2);
	f.add_term(std::make_shared<AutoDiffTerm<Rosenbrock, 2>>(), x);

	NewtonSolver solver;
	solver.log_function = nullptr;
	solver.maximum_iterations = 50;
	solver.function_improvement_tolerance = 0;
	solver.argument_improvement_tolerance = 0;
	SolverResults results;
	solver.solve(f, &results);

	EXPECT_TRUE(results.exit_condition == SolverResults::GRADIENT_TOLERANCE);
}

struct NanFunctor
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return std::numeric_limits<double>::quiet_NaN();
	}
};

struct InfFunctor
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return std::numeric_limits<double>::infinity();
	}
};

TEST(Solver, inf_nan)
{
	Function f_nan, f_inf;
	double x[1] = {-1.2};
	f_nan.add_variable(x, 1);
	f_inf.add_variable(x, 1);
	f_nan.add_term(std::make_shared<AutoDiffTerm<NanFunctor, 1>>(), x);
	f_inf.add_term(std::make_shared<AutoDiffTerm<InfFunctor, 1>>(), x);

	NewtonSolver solver;
	solver.log_function = nullptr;
	SolverResults results;

	solver.solve(f_nan, &results);
	EXPECT_EQ(results.exit_condition, SolverResults::FUNCTION_NAN);

	solver.solve(f_inf, &results);
	EXPECT_EQ(results.exit_condition, SolverResults::FUNCTION_INFINITY);
}

TEST(Solver, L_GBFS_exact)
{
	// Test that the L-BFGS solver follows a reference
	// MATLAB implementation (minFunc) configured to
	// use the same history size and line search method.

	std::vector<int>    iters;
	std::vector<double> fvals;
	iters.push_back(1); fvals.push_back(2.19820e+01);
	iters.push_back(2); fvals.push_back(4.96361e+00);
	iters.push_back(3); fvals.push_back(4.16118e+00);
	iters.push_back(4); fvals.push_back(4.09480e+00);
	iters.push_back(5); fvals.push_back(4.09165e+00);
	iters.push_back(6); fvals.push_back(3.87014e+00);
	iters.push_back(7); fvals.push_back(3.72822e+00);
	iters.push_back(8); fvals.push_back(3.45143e+00);
	iters.push_back(9); fvals.push_back(2.93307e+00);
	iters.push_back(10); fvals.push_back(2.45070e+00);
	iters.push_back(11); fvals.push_back(2.28498e+00);
	iters.push_back(12); fvals.push_back(1.96226e+00);
	iters.push_back(13); fvals.push_back(1.52784e+00);
	iters.push_back(14); fvals.push_back(1.33065e+00);
	iters.push_back(15); fvals.push_back(1.17817e+00);
	iters.push_back(16); fvals.push_back(9.82334e-01);
	iters.push_back(17); fvals.push_back(7.82560e-01);
	iters.push_back(18); fvals.push_back(6.26596e-01);
	iters.push_back(19); fvals.push_back(5.56740e-01);
	iters.push_back(20); fvals.push_back(4.76314e-01);
	iters.push_back(21); fvals.push_back(3.21285e-01);
	iters.push_back(22); fvals.push_back(2.91320e-01);
	iters.push_back(23); fvals.push_back(2.24196e-01);
	iters.push_back(24); fvals.push_back(1.72268e-01);
	iters.push_back(25); fvals.push_back(1.29991e-01);
	iters.push_back(26); fvals.push_back(9.11752e-02);
	iters.push_back(27); fvals.push_back(5.74927e-02);
	iters.push_back(28); fvals.push_back(3.14319e-02);
	iters.push_back(29); fvals.push_back(1.49973e-02);
	iters.push_back(30); fvals.push_back(9.20225e-03);
	iters.push_back(31); fvals.push_back(2.61646e-03);
	iters.push_back(32); fvals.push_back(6.34734e-04);
	iters.push_back(33); fvals.push_back(9.00566e-05);
	iters.push_back(34); fvals.push_back(7.38860e-06);
	iters.push_back(35); fvals.push_back(2.55965e-07);
	iters.push_back(36); fvals.push_back(3.40434e-10);

	for (int i = 0; i < iters.size(); ++i) {
		double x[2] = {-1.2, 1.0};
		Function f;
		f.add_variable(x, 2);
		f.add_term(std::make_shared<AutoDiffTerm<Rosenbrock, 2>>(), x);

		LBFGSSolver solver;
		SolverResults results;
		solver.log_function = nullptr;
		solver.lbfgs_history_size = 10;
		solver.maximum_iterations = iters[i];

		solver.solve(f, &results);
		double fval = f.evaluate();
		EXPECT_LE( std::abs(fval - fvals[i]) / std::abs(fval), 1e-4);
	}
}

TEST(Solver, Newton_exact)
{
	// Test that the Newton solver follows a reference
	// MATLAB implementation (minFunc) configured to
	// use the same line search method.

	// The Rosenbrock function has a positive definite
	// Hessian at eery iteration step, so this test does
	// not test the iterative Cholesky factorization.

	std::vector<int>    iters;
	std::vector<double> fvals;
	iters.push_back(1); fvals.push_back(4.73188e+000);
	iters.push_back(2); fvals.push_back(4.08740e+000);
	iters.push_back(3); fvals.push_back(3.22867e+000);
	iters.push_back(4); fvals.push_back(3.21390e+000);
	iters.push_back(5); fvals.push_back(1.94259e+000);
	iters.push_back(6); fvals.push_back(1.60019e+000);
	iters.push_back(7); fvals.push_back(1.17839e+000);
	iters.push_back(8); fvals.push_back(9.22412e-001);
	iters.push_back(9); fvals.push_back(5.97489e-001);
	iters.push_back(10); fvals.push_back(4.52625e-001);
	iters.push_back(11); fvals.push_back(2.80762e-001);
	iters.push_back(12); fvals.push_back(2.11393e-001);
	iters.push_back(13); fvals.push_back(8.90195e-002);
	iters.push_back(14); fvals.push_back(5.15354e-002);
	iters.push_back(15); fvals.push_back(1.99928e-002);
	iters.push_back(16); fvals.push_back(7.16924e-003);
	iters.push_back(17); fvals.push_back(1.06961e-003);
	iters.push_back(18); fvals.push_back(7.77685e-005);
	iters.push_back(19); fvals.push_back(2.82467e-007);
	iters.push_back(20); fvals.push_back(8.51707e-012);

	for (int i = 0; i < iters.size(); ++i) {
		double x[2] = {-1.2, 1.0};
		Function f;
		f.add_variable(x, 2);
		f.add_term(std::make_shared<AutoDiffTerm<Rosenbrock, 2>>(), x);

		NewtonSolver solver;
		solver.line_search_type = Solver::ARMIJO;

		SolverResults results;
		solver.log_function = nullptr;
		solver.maximum_iterations = iters[i];

		solver.solve(f, &results);
		double fval = f.evaluate();
		EXPECT_LE( std::abs(fval - fvals[i]) / std::abs(fval), 1e-5);
	}
}

TEST(Solver, Newton_exact_Wolfe)
{
	// Test that the Newton solver follows a reference
	// MATLAB implementation (minFunc) configured to
	// use the same line search method.

	// The Rosenbrock function has a positive definite
	// Hessian at eery iteration step, so this test does
	// not test the iterative Cholesky factorization.

	std::vector<double> fvals = {
		4.73188e+00,
		4.08740e+00,
		3.22867e+00,
		2.76783e+00,
		2.17872e+00,
		1.66968e+00,
		1.26823e+00,
		9.13472e-01,
		6.86341e-01,
		4.35423e-01,
		3.19604e-01,
		1.92120e-01,
		1.13097e-01,
		5.32981e-02,
		2.88735e-02,
		6.63610e-03,
		2.23143e-03,
		4.18107e-05,
		1.70026e-07,
		2.75421e-13};

	for (int i = 0; i < fvals.size(); ++i) {
		double x[2] = {-1.2, 1.0};
		Function f;
		f.add_variable(x, 2);
		f.add_term(std::make_shared<AutoDiffTerm<Rosenbrock, 2>>(), x);

		NewtonSolver solver;

		// MinFunc configuration.
		solver.line_search_type = Solver::WOLFE;
		solver.wolfe_interpolation_strategy = Solver::BISECTION;
		solver.line_search_c  = 1e-4;
		solver.line_search_c2 = 0.9;

		SolverResults results;
		solver.log_function = nullptr;
		solver.maximum_iterations = i + 1;

		solver.solve(f, &results);
		double fval = f.evaluate();
		EXPECT_LE( std::abs(fval - fvals[i]) / std::abs(fval), 1e-5);
	}
}

TEST(Solver, LBFGS_exact_Wolfe_far)
{
	// Test that the Newton solver follows a reference
	// MATLAB implementation (minFunc) configured to
	// use the same line search method.

	// The Rosenbrock function has a positive definite
	// Hessian at eery iteration step, so this test does
	// not test the iterative Cholesky factorization.

	std::vector<double> fvals = {
		1.46190e+14,
		3.41218e+13,
		1.24973e+13,
		3.86189e+12,
		1.27251e+12,
		4.07749e+11,
		1.31310e+11,
		4.17776e+10,
		1.31168e+10};

	for (int i = 0; i < fvals.size(); ++i) {
		double x[2] = {-1200, 1000};
		Function f;
		f.add_variable(x, 2);
		f.add_term(std::make_shared<AutoDiffTerm<Rosenbrock, 2>>(), x);

		LBFGSSolver solver;

		// MinFunc configuration.
		solver.line_search_type = Solver::WOLFE;
		solver.wolfe_interpolation_strategy = Solver::BISECTION;
		solver.line_search_c  = 1e-4;
		solver.line_search_c2 = 0.9;

		SolverResults results;
		solver.log_function = nullptr;
		solver.maximum_iterations = i + 1;

		solver.solve(f, &results);
		double fval = f.evaluate();
		EXPECT_LE( std::abs(fval - fvals[i]) / std::abs(fval), 1e-5);
	}
}

TEST_CASE("LBFGS_exact_Wolfe_interpolation")
{
	// Test that the Newton solver follows a reference
	// MATLAB implementation (minFunc) configured to
	// use the same line search method.

	// The Rosenbrock function has a positive definite
	// Hessian at eery iteration step, so this test does
	// not test the iterative Cholesky factorization.

	std::vector<double> fvals = {
		1.69372e+10,
		3.63800e+09,
		1.35617e+09,
		4.07136e+08,
		1.30647e+08,
		3.95499e+07,
		1.15617e+07,
		3.08730e+06,
		7.06478e+05,
		1.20698e+05,
		1.19546e+04,
		5.30431e+02,
		1.24938e+02,
		1.22820e+02,
		1.22820e+02,
		1.22819e+02,
		1.22818e+02,
		1.22816e+02,
		1.22808e+02,
		1.22790e+02,
		1.22741e+02,
		1.22610e+02,
		1.22244e+02,
		1.05808e+02,
		1.05502e+02,
		9.98985e+01,
		9.52870e+01,
		8.98113e+01,
		8.35851e+01,
		8.27050e+01,
		7.90132e+01,
		7.38046e+01,
		7.05325e+01,
		6.80851e+01,
		6.20812e+01};

	for (int i = 0; i < fvals.size(); ++i) {
		double x[2] = {-120, 100};
		Function f;
		f.add_variable(x, 2);
		f.add_term<AutoDiffTerm<Rosenbrock, 2>>(x);

		LBFGSSolver solver;
		solver.log_function = nullptr;

		// MinFunc configuration.
		solver.line_search_type = Solver::WOLFE;
		solver.wolfe_interpolation_strategy = Solver::CUBIC;
		solver.line_search_c  = 1e-4;
		solver.line_search_c2 = 0.9;

		SolverResults results;
		//solver.log_function = nullptr;
		solver.maximum_iterations = i + 1;

		solver.solve(f, &results);
		double fval = f.evaluate();
		CAPTURE(i)
		REQUIRE( (std::abs(fval - fvals[i]) / std::abs(fval)) < 1e-4);
	}
}

struct Quadratic2
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 = x[0] - 2.0;
		R d1 = x[1] + 7.0;
		return 2 * d0*d0 + d1*d1;
	}
};

struct Quadratic2Changed
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 = exp(x[0]) - 2.0;
		R d1 = exp(x[1]) + 7.0;
		return 2 * d0*d0 + d1*d1;
	}
};


//
//	x_i = exp(t_i)
//  t_i = log(x_i)
//
template<int dimension>
class ExpTransform
{
public:
	template<typename R>
	void t_to_x(R* x, const R* t) const
	{
		using std::exp;

		for (size_t i = 0; i < dimension; ++i) {
			x[i] = exp(t[i]);
		}
	}

	template<typename R>
	void x_to_t(R* t, const R* x) const
	{
		using std::log;

		for (size_t i = 0; i < dimension; ++i) {
			t[i] = log(x[i]);
		}
	}

	int x_dimension() const
	{
		return dimension;
	}

	int t_dimension() const
	{
		return dimension;
	}
};

TEST(Solver, SimpleConstraints)
{
	double x[2] = {1, 1};
	Function function;
	function.add_variable_with_change<ExpTransform<2>>(x, 2);
	function.add_term(std::make_shared<AutoDiffTerm<Quadratic2, 2>>(),
	                  x);

	double t[2] = {0, 0};
	Function function_changed;
	function_changed.add_variable(t, 2);
	function_changed.add_term(std::make_shared<AutoDiffTerm<Quadratic2Changed, 2>>(),
	                          t);

	NelderMeadSolver nm_solver;
	nm_solver.log_function = nullptr;
	
	LBFGSSolver lbfgs_solver;
	lbfgs_solver.log_function = nullptr;

	SolverResults results;
	results.exit_condition = SolverResults::NA;

	int max_iter = 1;
	while (! results.exit_success()) {
		nm_solver.maximum_iterations = max_iter++;
		lbfgs_solver.maximum_iterations = max_iter++;

		x[0] = x[1] = 1.0;
		t[0] = std::log(x[0]);
		t[1] = std::log(x[1]);
		nm_solver.solve(function, &results);
		nm_solver.solve(function_changed, &results);
		EXPECT_NEAR(function.evaluate(), function_changed.evaluate(), 1e-12);

		x[0] = x[1] = 1.0;
		t[0] = std::log(x[0]);
		t[1] = std::log(x[1]);
		lbfgs_solver.solve(function, &results);
		lbfgs_solver.solve(function_changed, &results);
		EXPECT_NEAR(function.evaluate(), function_changed.evaluate(), 1e-12);
	}

	EXPECT_LT( std::abs(x[0] - 2.0), 1e-6);
	EXPECT_LT( std::abs(x[1]), 1e-6);
}

TEST(Solver, PositiveConstraint)
{
	double x[2] = {1, 1};
	Function function;
	function.add_variable_with_change<GreaterThanZero>(x, 2, 2);
	function.add_term(
		std::make_shared<AutoDiffTerm<Quadratic2, 2>>(),
		x);

	LBFGSSolver solver;
	solver.log_function = nullptr;
	SolverResults results;
	solver.solve(function, &results);

	EXPECT_NEAR(x[0], 2.0, 1e-7);
	EXPECT_NEAR(x[1], 0.0, 1e-7);
}

TEST(Solver, BoxConstraint)
{
	double x[2] = {1, 1};
	Function function;
	double a[2] = {0.0, -0.5};
	double b[2] = {6.0, 10.0};
	function.add_variable_with_change<Box>(x, 2, 2, a, b);
	function.add_term(
		std::make_shared<AutoDiffTerm<Quadratic2, 2>>(),
		x);

	LBFGSSolver solver;
	solver.log_function = nullptr;
	SolverResults results;
	solver.solve(function, &results);

	EXPECT_NEAR(x[0],  2.0, 1e-4);
	EXPECT_NEAR(x[1], -0.5, 1e-4);
}

template<typename SolverClass>
void test_constant_variables()
{
	// This tests checks that the solver behaves identically when
	// extra variables are added and held constant

	SolverClass solver;
	solver.log_function = nullptr;
	SolverResults results;


	std::vector<double> v_normal, v_extra_1, v_extra_2;

	{
		double x[2] = {-1.2, 1.0};
		Function f;
		f.add_variable(x, 2);
		f.add_term(std::make_shared<AutoDiffTerm<Rosenbrock, 2>>(),
			        x);

		for (int i = 1; i <= 5; ++i) {
			solver.maximum_iterations = i;
			solver.solve(f, &results);
			v_normal.push_back(f.evaluate());
		}
	}

	// Add one extra variable.
	{
		double x[2] = {-1.2, 1.0};
		double y[1] = {0.0};
		Function f;
		f.add_variable(y, 1);
		f.add_variable(x, 2);
		f.add_term(std::make_shared<AutoDiffTerm<Rosenbrock, 2>>(),
			        x);

		f.set_constant(y, true);

		for (int i = 1; i <= 5; ++i) {
			solver.maximum_iterations = i;
			solver.solve(f, &results);
			v_extra_1.push_back(f.evaluate());
		}
	}

	// Add two extra variables.
	{
		double x[2] = {-1.2, 1.0};
		double y[1] = {0.0};
		double z[3] = {0, 0, 0};
		Function f;
		f.add_variable(y, 1);
		f.add_variable(x, 2);
		f.add_variable(z, 3);
		f.add_term(std::make_shared<AutoDiffTerm<Rosenbrock, 2>>(),
			        x);

		f.set_constant(y, true);
		f.set_constant(z, true);

		for (int i = 1; i <= 5; ++i) {
			solver.maximum_iterations = i;
			solver.solve(f, &results);
			v_extra_2.push_back(f.evaluate());
		}
	}

	// Check that the results are identical.
	for (int i = 0; i < v_normal.size(); ++i) {
		CHECK(v_normal.at(i) == v_extra_1.at(i));
		CHECK(v_normal.at(i) == v_extra_2.at(i));
	}
}

TEST(NewtonSolver, constant_variables)
{
	test_constant_variables<NewtonSolver>();
}

TEST(LBFGSSolver, constant_variables)
{
	test_constant_variables<LBFGSSolver>();
}

template<typename SolverClass>
void test_callback_function()
{
	Function f;
	double x[2] = { -1.2, 1.0 };
	f.add_term<AutoDiffTerm<Rosenbrock, 2>>(x);

	SolverClass solver;
	solver.callback_function = [&f](const CallbackInformation& information) -> bool
	{
		REQUIRE(information.x);
		CHECK(f.evaluate(*information.x) == information.objective_value);
		return false;
	};
	solver.log_function = nullptr;

	SolverResults results;
	solver.solve(f, &results);
	CHECK(results.exit_condition == SolverResults::USER_ABORT);
}

TEST(NewtonSolver, callback_function)
{
	test_callback_function<NewtonSolver>();
}

TEST(LBFGSSolver, callback_function)
{
	test_callback_function<LBFGSSolver>();
}

TEST(NelderMeadSolver, callback_function)
{
	test_callback_function<NelderMeadSolver>();
}


template<typename SolverClass>
void test_empty_function_crash_bug()
{
	SolverClass solver;
	solver.log_function = nullptr;
	Function function;
	SolverResults results;
	solver.solve(function, &results);
	CHECK(results.exit_condition == SolverResults::FUNCTION_TOLERANCE);
}

TEST(NewtonSolver, empty_function_crash_bug)
{
	test_empty_function_crash_bug<NewtonSolver>();
}

TEST(LBFGSSolver, empty_function_crash_bug)
{
	test_empty_function_crash_bug<LBFGSSolver>();
}

TEST(NelderMeadSolver, empty_function_crash_bug)
{
	test_empty_function_crash_bug<NelderMeadSolver>();
}

TEST(PatternSolver, empty_function_crash_bug)
{
	test_empty_function_crash_bug<PatternSolver>();
}

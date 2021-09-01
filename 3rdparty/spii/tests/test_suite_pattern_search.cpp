// Petter Strandmark 2012
//
#include <cmath>
#include <iostream>
#include <random>

#include <catch.hpp>
#include <spii/google_test_compatibility.h>

#include <spii/auto_diff_term.h>
#include <spii/solver.h>

using namespace spii;

void info_log_function(const std::string& str)
{
	INFO(str);
}

std::unique_ptr<Solver> create_solver()
{
	std::unique_ptr<PatternSolver> solver(new PatternSolver);

	solver->maximum_iterations = 500000;
	solver->area_tolerance = 1e-18;
	solver->function_improvement_tolerance = 1e-12;

	solver->log_function = info_log_function;

	return std::move(solver);
}

template<typename Functor, int dimension>
double run_test(double* var, const Solver* solver = 0)
{
	Function f;

	f.add_variable(var, dimension);
	f.add_term(std::make_shared<AutoDiffTerm<Functor, dimension>>(), var);

	auto own_solver = create_solver();
	if (solver == 0) {
		solver = own_solver.get();
	}

	SolverResults results;
	solver->solve(f, &results);
	INFO(results);

	std::stringstream sout;
	for (int i = 0; i < dimension; ++i) {
		sout << "x" << i + 1 << " = " << var[i] << ",  ";
	}
	INFO(sout.str());

	EXPECT_TRUE(results.exit_condition == SolverResults::GRADIENT_TOLERANCE ||
	            results.exit_condition == SolverResults::FUNCTION_TOLERANCE);

	return f.evaluate();
}


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

TEST(Solver, Rosenbrock)
{
	double x[2] = {-1.2, 1.0};
	double fval = run_test<Rosenbrock, 2>(x);

	EXPECT_LT( std::fabs(x[0] - 1.0), 1e-9);
	EXPECT_LT( std::fabs(x[1] - 1.0), 1e-9);
	EXPECT_LT( std::fabs(fval), 1e-9);
}

TEST(Solver, RosenbrockFar)
{
	double x[2] = {-1e6, 1e6};

	auto solver = create_solver();
	solver->gradient_tolerance = 1e-40;
	solver->maximum_iterations = 100000;
	double fval = run_test<Rosenbrock, 2>(x, solver.get());

	EXPECT_LT( std::fabs(x[0] - 1.0), 1e-9);
	EXPECT_LT( std::fabs(x[1] - 1.0), 1e-9);
	EXPECT_LT( std::fabs(fval), 1e-9);
}

struct Colville
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return 100.0 * (x[1] - x[0]*x[0]) * (x[1] - x[0]*x[0])
		+ (1.0 - x[0]) * (1.0 - x[0])
		+ 90.0 * (x[3] - x[2]*x[2]) * (x[3] - x[2]*x[2])
		+ (1.0 - x[2]) * (1.0 - x[2])
		+ 10.1 * ( (x[1] - 1.0 ) * (x[1] - 1.0)
			+ (x[3] - 1.0) * (x[3] - 1.0) )
		+ 19.8 * (x[1] - 1.0) * (x[3] - 1.0);
	}
};

TEST(Solver, Colville)
{
	double x[4] = {-0.5, 1.0, -0.5, -1.0};
	run_test<Colville, 4>(x);

	EXPECT_LT( std::fabs(x[0] - 1.0), 1e-8);
	EXPECT_LT( std::fabs(x[1] - 1.0), 1e-8);
	EXPECT_LT( std::fabs(x[2] - 1.0), 1e-8);
	EXPECT_LT( std::fabs(x[3] - 1.0), 1e-8);
}

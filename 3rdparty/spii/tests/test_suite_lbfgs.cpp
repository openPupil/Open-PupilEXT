// Petter Strandmark 2012
//
// Test functions from
// Jorge J. More, Burton S. Garbow and Kenneth E. Hillstrom,
// "Testing unconstrained optimization software",
// Transactions on Mathematical Software 7(1):17-41, 1981.
// http://www.caam.rice.edu/~zhang/caam454/nls/MGH.pdf
//
#include <cmath>
#include <iostream>
#include <random>

#include <catch.hpp>
#include <spii/google_test_compatibility.h>

#include <spii/auto_diff_term.h>
#include <spii/solver.h>

using namespace spii;

std::stringstream global_string_stream;
void info_log_function(const std::string& str)
{
	global_string_stream << str << "\n";
}

std::unique_ptr<Solver> create_solver()
{
	std::unique_ptr<LBFGSSolver> solver(new LBFGSSolver);

	solver->maximum_iterations = 1000;

	solver->function_improvement_tolerance = 0;
	solver->argument_improvement_tolerance = 0;
	solver->gradient_tolerance = 1e-7;
	
	solver->lbfgs_history_size = 40;

	solver->log_function = info_log_function;

	return std::move(solver);
}

template<typename Functor, int dimension>
double run_test(double* var, const Solver* solver = 0)
{
	Function f;
	f.hessian_is_enabled = false;

	f.add_variable(var, dimension);
	f.add_term(std::make_shared<AutoDiffTerm<Functor, dimension>>(), var);

	auto own_solver = create_solver();
	if (solver == 0) {
		solver = own_solver.get();
	}

	SolverResults results;
	global_string_stream.str("");
	solver->solve(f, &results);
	INFO(global_string_stream.str());
	INFO(results);

	std::stringstream sout;
	f.print_timing_information(sout);
	for (int i = 0; i < dimension; ++i) {
		sout << "x" << i + 1 << " = " << var[i] << ",  ";
	}
	INFO(sout.str());

	EXPECT_TRUE(results.exit_condition == SolverResults::GRADIENT_TOLERANCE);

	return f.evaluate();
}

#include "suite_more_et_al.h"
#include "suite_test_opt.h"
#include "suite_uctp.h"


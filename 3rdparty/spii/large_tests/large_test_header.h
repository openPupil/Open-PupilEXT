//
// Provides a run_test function for running large tests.
//

#include <cmath>
#include <functional>
#include <iostream>
#include <random>
#include <vector>

#include <spii/auto_diff_term.h>
#include <spii/solver.h>

using namespace spii;

template<typename SolverClass>
void run_test_main(const std::function<void(std::vector<double>&, Function*)>& create_f,
                   const std::function<std::vector<double>(int)>& start, 
                   int n)
{
	auto this_start = start(n);
	Function f;
	create_f(this_start, &f);
	REQUIRE(f.get_number_of_scalars() == this_start.size());

	std::stringstream information_stream;

	SolverClass solver;
	solver.log_function =
		[&information_stream](const std::string& str)
		{
			information_stream << str << "\n";
		};
	solver.function_improvement_tolerance = 0;
	solver.argument_improvement_tolerance = 0;
	solver.gradient_tolerance = 1e-7;
	solver.maximum_iterations = 10000;

	if (typeid(SolverClass) == typeid(NewtonSolver)) {
		solver.maximum_iterations = 1000;
	}

	SolverResults results;
	solver.solve(f, &results);

	f.print_timing_information(information_stream);
	INFO(information_stream.str());
	INFO(results);

	CHECK(results.exit_success());
}

void run_test(const std::function<void(std::vector<double>&, Function*)>& create_f,
              const std::function<std::vector<double>(int)>& start,
              bool test_newton = true)
{
	if (test_newton) {
		SECTION("Newton-100", "") {
			run_test_main<NewtonSolver>(create_f, start, 100);
		}
		SECTION("Newton-1000", "") {
			run_test_main<NewtonSolver>(create_f, start, 1000);
		}
		SECTION("Newton-10000", "") {
			run_test_main<NewtonSolver>(create_f, start, 10000);
		}
	}

	SECTION("LBFGS-100", "") {
		run_test_main<LBFGSSolver>(create_f, start, 100);
	}
	SECTION("LBFGS-1000", "") {
		run_test_main<LBFGSSolver>(create_f, start, 1000);
	}
	SECTION("LBFGS-10000", "") {
		run_test_main<LBFGSSolver>(create_f, start, 10000);
	}
}

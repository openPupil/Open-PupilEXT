// Petter Strandmark 2013.

#include <queue>

#include <catch.hpp>

#include <spii/interval_term.h>
#include <spii/solver.h>
using namespace spii;

struct SimpleFunction1
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return pow(x[0], 2) + 3.0*cos(5.0*x[0]);
	}
};

struct SimpleFunction2
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return x[0]*x[0] + x[1]*x[1] + 1.0;
	}
};

struct SimpleFunction1_1
{
	template<typename R>
	R operator()(const R* const x, const R* const y) const
	{
		return x[0]*x[0] + y[0]*y[0] + 1.0;
	}
};

TEST_CASE("global_optimization/simple_function1", "Petter")
{
	double x = 2.0;
	Function f;
	f.add_variable(&x, 1);
	f.add_term(std::make_shared<IntervalTerm<SimpleFunction1, 1>>(),
	           &x);

	GlobalSolver solver;
	solver.maximum_iterations = 1000;
	solver.function_improvement_tolerance = 1e-5;
	std::stringstream info_buffer;
	solver.log_function = [&info_buffer](const std::string& str) { info_buffer << str << std::endl; };
	SolverResults results;
	std::vector<Interval<double>> x_interval;
	x_interval.push_back(Interval<double>(-10.0, 9.0));
	auto interval = solver.solve_global(f, x_interval, &results);
	REQUIRE(interval.size() == 1);

	f.print_timing_information(info_buffer);
	INFO(info_buffer.str());
	INFO(results);

	CHECK(results.exit_success());
	auto opt = Interval<double>(results.optimum_lower, results.optimum_upper);
	CHECK((opt.get_upper() - opt.get_lower()) <= 1e-3);
	auto val = f.evaluate();
	CHECK(opt.get_lower() <= val); CHECK(val <= opt.get_upper());
}

TEST_CASE("global_optimization/simple_function2", "Petter")
{
	double x[] = {2.0, 2.0};
	Function f;
	f.add_variable(x, 2);
	f.add_term(std::make_shared<IntervalTerm<SimpleFunction2, 2>>(),
	           x);

	GlobalSolver solver;
	solver.maximum_iterations = 1000;
	solver.argument_improvement_tolerance = 0;
	solver.function_improvement_tolerance = 1e-12;
	std::stringstream info_buffer;
	solver.log_function = [&info_buffer](const std::string& str) { info_buffer << str << std::endl; };
	SolverResults results;
	std::vector<Interval<double>> x_interval;
	x_interval.push_back(Interval<double>(-10.0, 9.0));
	x_interval.push_back(Interval<double>(-8.0, 8.0));
	auto interval = solver.solve_global(f, x_interval, &results);
	REQUIRE(interval.size() == 2);

	f.print_timing_information(info_buffer);
	INFO(info_buffer.str());
	INFO(results);
	CHECK(results.exit_success());

	auto opt = Interval<double>(results.optimum_lower, results.optimum_upper);
	CHECK((opt.get_upper() - opt.get_lower()) <= 1e-10);
	CHECK(results.exit_condition == SolverResults::FUNCTION_TOLERANCE);
	double ground_truth = 1.0;
	CHECK(opt.get_lower() <= ground_truth); CHECK(ground_truth <= opt.get_upper());
}

TEST_CASE("global_optimization/simple_function1-1", "Petter")
{
	double x = 2.0, y = 2.0;
	Function f;
	f.add_variable(&x, 1);
	f.add_variable(&y, 1);
	f.add_term(std::make_shared<IntervalTerm<SimpleFunction1_1, 1, 1>>(),
	           &x, &y);

	GlobalSolver solver;
	solver.maximum_iterations = 1000;
	solver.argument_improvement_tolerance = 0;
	solver.function_improvement_tolerance = 1e-12;
	std::stringstream info_buffer;
	solver.log_function = [&info_buffer](const std::string& str) { info_buffer << str << std::endl; };
	SolverResults results;
	std::vector<Interval<double>> x_interval;
	x_interval.push_back(Interval<double>(-10.0, 9.0));
	x_interval.push_back(Interval<double>(-8.0, 8.0));
	auto interval = solver.solve_global(f, x_interval, &results);
	REQUIRE(interval.size() == 2);

	INFO(info_buffer.str());
	INFO(results);
	CHECK(results.exit_success());

	auto opt = Interval<double>(results.optimum_lower, results.optimum_upper);
	CHECK((opt.get_upper() - opt.get_lower()) <= 1e-10);
	CHECK(results.exit_condition == SolverResults::FUNCTION_TOLERANCE);
	double ground_truth = 1.0;
	CHECK(opt.get_lower() <= ground_truth); CHECK(ground_truth <= opt.get_upper());
}

TEST_CASE("Constant variable")
{
	double x = 2.0;
	double y = 2.0;
	Function f;
	f.add_variable(&x, 1);
	f.add_variable(&y, 1);
	f.add_term(std::make_shared<IntervalTerm<SimpleFunction1_1, 1, 1>>(),
	           &x, &y);

	GlobalSolver solver;
	solver.maximum_iterations = 1000;
	solver.argument_improvement_tolerance = 0;
	solver.function_improvement_tolerance = 1e-12;

	{
		f.set_constant(&y, true);
		REQUIRE(f.get_number_of_variables() == 2);
		REQUIRE(f.get_number_of_scalars() == 1);

		std::stringstream info_buffer;
		solver.log_function = [&info_buffer](const std::string& str) { info_buffer << str << std::endl; };
		SolverResults results;
		std::vector<Interval<double>> x_interval;
		x_interval.push_back(Interval<double>(-10.0, 9.0));

		auto interval = solver.solve_global(f, x_interval, &results);
		REQUIRE(interval.size() == 1);
		INFO(info_buffer.str());
		INFO(results);

		auto opt = Interval<double>(results.optimum_lower, results.optimum_upper);
		CHECK((opt.get_upper() - opt.get_lower()) <= 1e-10);
		CHECK(results.exit_condition == SolverResults::FUNCTION_TOLERANCE);
		double ground_truth = 5.0;
		CHECK(opt.get_lower() <= ground_truth); CHECK(ground_truth <= opt.get_upper());
	}

	{
		f.set_constant(&y, false);
		x = 3.0;
		f.set_constant(&x, true);
		REQUIRE(f.get_number_of_variables() == 2);
		REQUIRE(f.get_number_of_scalars() == 1);

		std::stringstream info_buffer;
		solver.log_function = [&info_buffer](const std::string& str) { info_buffer << str << std::endl; };

		SolverResults results;
		std::vector<Interval<double>> x_interval;
		x_interval.push_back(Interval<double>(-10.0, 10.0));
		auto interval = solver.solve_global(f, x_interval, &results);
		REQUIRE(interval.size() == 1);
		INFO(info_buffer.str());
		INFO(results);

		Interval<double> opt(results.optimum_lower, results.optimum_upper);
		CHECK((opt.get_upper() - opt.get_lower()) <= 1e-10);
		CHECK(results.exit_condition == SolverResults::FUNCTION_TOLERANCE);
		double ground_truth = 10.0;
		CHECK(opt.get_lower() <= ground_truth); CHECK(ground_truth <= opt.get_upper());
	}
}

template<typename Functor, int dimension>
void run_test(double* x,
              double distance_from_start = 1.0,
              double x_maximum_gap = 1e-5,
              double ground_truth = std::numeric_limits<double>::quiet_NaN(),
              double maximum_gap = 1e-10,
              double function_improvement_tolerance = 1e-12)
{
	using namespace std;

	Function f;
	f.add_variable(x, dimension);
	f.add_term<IntervalTerm<Functor, dimension>>(x);

	GlobalSolver solver;
	solver.maximum_iterations = 1000000;
	solver.argument_improvement_tolerance = 0;
	solver.function_improvement_tolerance = function_improvement_tolerance;
	stringstream info_buffer;
	solver.log_function = [&info_buffer](const std::string& str) { info_buffer << str << std::endl; };
	SolverResults results;

	vector<Interval<double>> x_interval;
	for (int i = 0; i < dimension; ++i) {
		x_interval.push_back(Interval<double>(-0.784 - distance_from_start + x[i],  x[i] + distance_from_start + 0.1868));
	}
	auto interval = solver.solve_global(f, x_interval, &results);
	INFO(info_buffer.str());
	INFO(results);
	stringstream fout;
	f.print_timing_information(fout);
	INFO(fout.str());
	REQUIRE(interval.size() == dimension);

	auto opt = Interval<double>(results.optimum_lower, results.optimum_upper);
	CHECK((opt.get_upper() - opt.get_lower()) <= maximum_gap);
	CHECK(results.exit_condition == SolverResults::FUNCTION_TOLERANCE);

	if (ground_truth == ground_truth) {  // Tests for nan.
		CHECK(opt.get_lower() <= ground_truth); CHECK(ground_truth <= opt.get_upper());
	}

	for (int i = 0; i < dimension; ++i) {
		CHECK(interval[i].length() <= x_maximum_gap);
	}
}

struct Rosenbrock
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 =  x[1] - x[0]*x[0];
		R d1 =  1.0 - x[0];
		// Add a constant to make optimum not equal to 0.
		// If optimum is 0, everything works except that the notion
		// of relative interval size is not very useful.
		return 100.0 * d0*d0 + d1*d1 + 42.0;
	}
};

TEST_CASE("Rosenbrock")
{
	double x[] = {2.0, 2.0};
	run_test<Rosenbrock, 2>(x, 1.0, 1e-4);
	CHECK(std::abs(x[0] - 1.0) <= 1e-6);
	CHECK(std::abs(x[1] - 1.0) <= 1e-6);
}

struct FreudenStein_Roth
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 =  -13.0 + x[0] + ((5.0 - x[1])*x[1] - 2.0)*x[1];
		R d1 =  -29.0 + x[0] + ((x[1] + 1.0)*x[1] - 14.0)*x[1];
		// Add a constant to make optimum not equal to 0.
		// If optimum is 0, everything works except that the notion
		// of relative interval size is not very useful.
		return d0*d0 + d1*d1 + 42.0;
	}
};

TEST_CASE("FreudenStein_Roth")
{
	double x[2] = {0.5, -2.0};
	run_test<FreudenStein_Roth, 2>(x, 10.0, 1e-4, 42.0);
	CHECK(std::abs(x[0] - 5.0) <= 1e-5);
	CHECK(std::abs(x[1] - 4.0) <= 1e-5);

	// test_suite_newton can end up in local minima 48.9842...
}

struct Beale
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 = 1.5   - x[0] * (1.0 - x[1]);
		R d1 = 2.25  - x[0] * (1.0 - x[1]*x[1]);
		R d2 = 2.625 - x[0] * (1.0 - x[1]*x[1]*x[1]);
		// Add a constant to make optimum not equal to 0.
		// If optimum is 0, everything works except that the notion
		// of relative interval size is not very useful.
		return d0*d0 + d1*d1 + d2*d2 + 42.0;
	}
};

TEST_CASE("Beale")
{
	double x[2] = {1.0, 1.0};
	run_test<Beale, 2>(x, 10.0, 1e-4, 42.0, 1e-8);

	CHECK( std::fabs(x[0] - 3.0) <=  1e-5);
	CHECK( std::fabs(x[1] - 0.5) <= 1e-5);
}


struct Wood
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R f1 = 10.0 * (x[1] - x[0]*x[0]);
		R f2 = 1.0 - x[0];
		R f3 = sqrt(90.0) * (x[3] - x[2]*x[2]);
		R f4 = 1.0 - x[2];
		R f5 = sqrt(10.0) * (x[1] + x[3] - 2.0);
		R f6 = 1.0 / sqrt(10.0) * (x[1] - x[3]);
		// Add a constant to make optimum not equal to 0.
		// If optimum is 0, everything works except that the notion
		// of relative interval size is not very useful.
		return f1*f1 + f2*f2 + f3*f3 + f4*f4 + f5*f5 + f6*f6 + 42.0;
	}
};

TEST_CASE("Wood")
{
	double x[4] = {-3.0, -1.0, -3.0, -1.0};
	run_test<Wood, 4>(x, 5.0, 1e-2, 42.0);

	CHECK( std::fabs(x[0] - 1.0) <= 1e-5);
	CHECK( std::fabs(x[1] - 1.0) <= 1e-5);
	CHECK( std::fabs(x[2] - 1.0) <= 1e-5);
	CHECK( std::fabs(x[3] - 1.0) <= 1e-5);
}

struct BraninRCOS
{
	template<typename R>
	R operator()(const R* const x) const
	{
		const double pi = 3.141592653589793;
		const double a  = 1.0;
		const double d  = 6.0;
		const double e  = 10.0;
		const double b  = 5.1 / ( 4.0 * pi*pi );
		const double c  = 5.0 / pi;
		const double ff = 1.0 / ( 8.0 * pi );

		R expr = ( x[1] - b * x[0]*x[0] + c * x[0] - d );
		return a * expr * expr
			+ e * ( 1.0 - ff ) * cos ( x[0] ) + e;
	}
};

TEST_CASE("BraninRCOS")
{
	// From http://www.staff.brad.ac.uk/jpli/research/scga/function/branin_rcos.htm :
	// Has three solutions.
	// f(x1,x2)=0.397887; (x1,x2)=(-pi,12.275), (pi,2.275), (9.42478,2.475).

	// const double optimum = 0.3978873577;

	{
		double x[2] = {-4.0, 11.0};
		run_test<BraninRCOS, 2>(x, 2.0, 1e-5);
		CHECK( std::fabs(x[0] + 3.141592653589793) <= 1e-5);
		CHECK( std::fabs(x[1] - 12.275) <= 1e-4);
	}

	{
		double x[2] = {4.1, 1.0};
		run_test<BraninRCOS, 2>(x, 2.0, 1e-5);
		CHECK( std::fabs(x[0] - 3.141592653589793) <= 1e-5);
		CHECK( std::fabs(x[1] - 2.275) <= 1e-4);
	}

	{
		double x[2] = {8.0, 3.0};
		run_test<BraninRCOS, 2>(x, 2.0, 1e-5);
		CHECK( std::fabs(x[0] - 9.42478) <= 1e-5);
		CHECK( std::fabs(x[1] - 2.475) <= 1e-5);
	}
}

struct PowellSingular
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 = x[0] + 10.0 * x[1];
		R d1 = sqrt(5.0) * (x[2] - x[3]);
		R d2 = x[1] - 2.0*x[3];
		d2 = d2 * d2;
		R d3 = sqrt(10.0) * (x[0] - x[3]) * (x[0] - x[3]);
		return d0*d0 + d1*d1 + d2*d2 + d3*d3 + 42.0;
	}
};

TEST_CASE("PowellSingular")
{
	using namespace std;

	// With four variables, convergence can already be quite slow.
	double x[4] = {3.0, -1.0, 0.0, 1.0};
	run_test<PowellSingular, 4>(x, 4.0, 2e-1, 42.0, 1e-4, 1e-6);

	CHECK(abs(x[0]) <= 1e-1);
	CHECK(abs(x[1]) <= 1e-1);
	CHECK(abs(x[2]) <= 1e-1);
	CHECK(abs(x[3]) <= 1e-1);
}

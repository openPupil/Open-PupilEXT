// Petter Strandmark 2013.

#include <catch.hpp>

#include <spii/auto_diff_term.h>
#include <spii/constrained_function.h>
#include <spii/solver.h>

using namespace spii;
using namespace std;

class ObjectiveTerm
{
public:
	template<typename R>
	R operator()(const R* const x) const
	{
		auto dx = x[0] - 2.0;
		auto dy = x[1] - 2.0;
		return  dx*dx + dy*dy;
	}
};

//  x·x + y·y ≤ 1
class ConstraintTerm
{
public:
	template<typename R>
	R operator()(const R* const x) const
	{
		auto dx = x[0];
		auto dy = x[1];
		return  dx*dx + dy*dy - 1;
	}
};

//  x ≤ 100
class LessThan100
{
public:
	template<typename R>
	R operator()(const R* const x) const
	{
		auto dx = x[0];
		return  dx - 100;
	}
};

void test_simple_constrained_function(const std::vector<double> x_start, bool feasible)
{
	ConstrainedFunction function;
	stringstream log_stream;
	LBFGSSolver solver;
	solver.log_function =
		[&log_stream](const string& str)
		{
			log_stream << str << endl;
		};

	auto x = x_start;

	function.add_term(make_shared<AutoDiffTerm<ObjectiveTerm, 2>>(), &x[0]);
	function.add_constraint_term("circle", make_shared<AutoDiffTerm<ConstraintTerm, 2>>(), &x[0]);
	function.add_constraint_term("less than 100", make_shared<AutoDiffTerm<LessThan100, 2>>(), &x[0]);
	CHECK(function.is_feasible() == feasible);
	
	SolverResults results;
	function.solve(solver, &results);
	log_stream << results << endl;

	INFO(log_stream.str());
	CAPTURE(x[0]);
	CAPTURE(x[1]);
	CHECK((x[0]*x[0] + x[1]*x[1]) <= (1.0 + 1e-8));

	auto dx = x[0] - 2;
	auto dy = x[1] - 2;
	CHECK(abs(function.objective().evaluate() - (dx*dx + dy*dy)) <= 1e-14);
}

TEST_CASE("feasible_start")
{
	test_simple_constrained_function({0.0, 0.0}, true);
}


TEST_CASE("infeasible_start")
{
	test_simple_constrained_function({10.0, 4.0}, false);
}

TEST_CASE("max_iterations")
{
	ConstrainedFunction function;
	function.max_number_of_iterations = 1;
	vector<double> x = {0, 0};
	function.add_term(make_shared<AutoDiffTerm<ObjectiveTerm, 2>>(), &x[0]);
	function.add_constraint_term("circle", make_shared<AutoDiffTerm<ConstraintTerm, 2>>(), &x[0]);
	function.add_constraint_term("less than 100", make_shared<AutoDiffTerm<LessThan100, 2>>(), &x[0]);
	LBFGSSolver solver;
	solver.log_function = nullptr;
	SolverResults results;
	function.solve(solver, &results);
	CHECK(results.exit_condition == SolverResults::NO_CONVERGENCE);
}

TEST_CASE("no_readding_constraints")
{
	ConstrainedFunction function;
	double x[2] = {0, 0};
	function.add_constraint_term("circle", make_shared<AutoDiffTerm<ConstraintTerm, 2>>(), &x[0]);
	CHECK_THROWS(function.add_constraint_term("circle", make_shared<AutoDiffTerm<ConstraintTerm, 2>>(), &x[0]));
}

//  x = 100
class EqualTo100
{
public:
	template<typename R>
	R operator()(const R* const x) const
	{
		auto dx = x[0];
		return  dx - 100;
	}
};

TEST_CASE("feasible_equality")
{
	ConstrainedFunction function;
	double x = 0;
	function.add_equality_constraint_term("equal to 100", make_shared<AutoDiffTerm<EqualTo100, 1>>(), &x);
	CHECK(!function.is_feasible());
	x = 200;
	CHECK(!function.is_feasible());
	x = 100;
	CHECK(function.is_feasible());
}


// a·x + b·y - c
class Line
{
public:
	Line(double a_, double b_, double c_)
		: a{a_}, b{b_}, c{c_}
	{ }

	template<typename R>
	R operator()(const R* const x) const
	{
		return  a*x[0] + b*x[1] - c;
	}
private:
	double a, b, c;
};

TEST_CASE("Two_lines_intersect")
{
	vector<pair<double, double>> start_values =
		{{3., 4.}, {10., 20.}, {0., 0.1}, {-4., -9.}};

	for (auto start : start_values) {
		ConstrainedFunction function;

		// Infeasible start.
		vector<double> x(3);
		x[0] = start.first;
		x[1] = start.second;
		CAPTURE(x[0]);
		CAPTURE(x[1]);

		function.add_term<AutoDiffTerm<ObjectiveTerm, 2>>(&x[0]);
		function.add_equality_constraint_term("Line1", make_shared<AutoDiffTerm<Line, 2>>(4, 1, 0), &x[0]);
		function.add_equality_constraint_term("Line2", make_shared<AutoDiffTerm<Line, 2>>(-1, -5, 1), &x[0]);

		LBFGSSolver solver;
		stringstream sout;
		solver.log_function = [&sout](const string& str)
		                        { sout << str << endl; };
		SolverResults results;

		function.solve(solver, &results);
		sout << results << endl;
		INFO(sout.str());
		CAPTURE(x[0]);
		CAPTURE(x[1]);
		CHECK(results.exit_condition == SolverResults::GRADIENT_TOLERANCE);
		CHECK(function.is_feasible());
	}
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

TEST_CASE("Always_feasible")
{
	ConstrainedFunction function;

	// Always feasible and should converge after one
	// minimization.
	function.max_number_of_iterations = 1;

	stringstream log_stream;
	
	LBFGSSolver solver;
	solver.log_function =
		[&log_stream](const string& str)
		{
			log_stream << str << endl;
		};

	vector<double> x = {0, 0};

	function.add_term(make_shared<AutoDiffTerm<Rosenbrock, 2>>(), x.data());
	function.add_constraint_term("x[0] less than 100", make_shared<AutoDiffTerm<LessThan100, 2>>(), &x[0]);
	REQUIRE(function.is_feasible());
	
	SolverResults results;
	function.solve(solver, &results);
	function.objective().print_timing_information(log_stream);
	log_stream << results << endl;

	INFO(log_stream.str());
	CAPTURE(x[0]);
	CAPTURE(x[1]);
	CHECK(abs(function.objective().evaluate()) < 1e-8);
	CHECK(results.exit_condition == SolverResults::GRADIENT_TOLERANCE);
	CHECK(function.is_feasible());
}

//  x·x + y·y ≤ -1
class NormLessThanMinus1
{
public:
	template<typename R>
	R operator()(const R* const x) const
	{
		auto dx = x[0];
		auto dy = x[1];
		return  dx*dx + dy*dy + 1;
	}
};

TEST_CASE("Infeasible_problem")
{
	ConstrainedFunction function;

	function.max_number_of_iterations = 20;

	stringstream log_stream;
	
	LBFGSSolver solver;
	solver.log_function =
		[&log_stream](const string& str)
		{
			log_stream << str << endl;
		};

	vector<double> x = {0, 0};

	function.add_term(make_shared<AutoDiffTerm<Rosenbrock, 2>>(), x.data());
	function.add_constraint_term("Negative norm", make_shared<AutoDiffTerm<NormLessThanMinus1, 2>>(), x.data());
	REQUIRE(!function.is_feasible());
	
	SolverResults results;
	function.solve(solver, &results);
	function.objective().print_timing_information(log_stream);
	log_stream << results << endl;

	INFO(log_stream.str());
	CAPTURE(x[0]);
	CAPTURE(x[1]);
	CHECK(results.exit_condition == SolverResults::NO_CONVERGENCE);
	CHECK(!function.is_feasible());
}

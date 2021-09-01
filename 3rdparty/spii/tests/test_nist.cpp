// Petter Strandmark 2013
//
// This is a test suite of non-linear least-squares problems
// from NIST.
// http://www.itl.nist.gov/div898/strd/nls/nls_main.shtml
//
// The code loading the NIST data files has been adapted from
// Ceres, see http://code.google.com/ceres-solver .
//
// Note: g++ 4.5.3 gives array out of bounds warnings for
// specializations in auto_diff_term.h. As far as I can tell,
// these specializations are never executed and the warnings
// disappear when the number of tests change.
// 

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

#include <Eigen/Dense>

#include <catch.hpp>

#include <spii/auto_diff_term.h>
#include <spii/solver.h>

using namespace spii;

std::ofstream output_file("nist.log");

void skip_lines(std::istream* in, int num_lines)
{
	std::string str;
	for (int i = 0; i < num_lines; ++i) {
		std::getline(*in, str);
	}
}

void split_string(const std::string& str, std::vector<std::string>* tokens)
{
	std::stringstream sin(str);
	tokens->clear();
	while (true) {
		std::string s;
		sin >> s;
		if (sin) {
			tokens->push_back(s);
		}
		else {
			break;
		}
	}
}

template<typename T>
T convert(const std::string& str)
{
	std::stringstream sin(str);
	T t;
	sin >> t;
	if (!sin) {
		throw std::runtime_error("Conversion failed.");
	}
	return t;
}

void get_and_split_line(std::istream* in, std::vector<std::string>* tokens)
{
	std::string str;
	std::getline(*in, str);
	split_string(str, tokens);
}

class NISTProblem
{
public:
	NISTProblem(std::string filename)
	{
		std::ifstream fin(filename);
		if (!fin) {
			// Perhaps we are running the command from the root
			// project folder.
			filename = "bin/" + filename;
			fin.open(filename);
			if (!fin) {
				// Perhaps we are running the command from the test
				// project folder.
				filename = "../" + filename;
				fin.open(filename);
				if (!fin) {
					std::string error = "Failed to open ";
					error += filename;
					throw std::runtime_error(error.c_str());
				}
			}
		}
		std::vector<std::string> tokens;

		skip_lines(&fin, 24);
		get_and_split_line(&fin, &tokens);
		const int num_responses = convert<int>(tokens.at(1));

		get_and_split_line(&fin, &tokens);
		const int num_predictors = convert<int>(tokens.at(0));

		get_and_split_line(&fin, &tokens);
		const int num_observations = convert<int>(tokens.at(0));

		skip_lines(&fin, 4);
		get_and_split_line(&fin, &tokens);
		const int num_parameters = convert<int>(tokens.at(0));
		skip_lines(&fin, 8);

		get_and_split_line(&fin, &tokens);
		const int num_tries = static_cast<int>(tokens.size() - 4);

		this->predictor.resize(num_observations, num_predictors);
		this->response.resize(num_observations, num_responses);
		this->initial_parameters.resize(num_tries, num_parameters);
		this->final_parameters.resize(1, num_parameters);

		int parameter_id = 0;
		for (int i = 0; i < num_tries; ++i) {
			this->initial_parameters(i, parameter_id) =
				convert<double>(tokens.at(i + 2));
		}
		final_parameters(0, parameter_id) =
			convert<double>(tokens.at(2 + num_tries));

		for (parameter_id = 1; parameter_id < num_parameters; ++parameter_id) {
			get_and_split_line(&fin, &tokens);
				for (int i = 0; i < num_tries; ++i) {
				this->initial_parameters(i, parameter_id) =
					convert<double>(tokens.at(i + 2));
			}
			final_parameters(0, parameter_id) =
				convert<double>(tokens.at(2 + num_tries));
		}

		skip_lines(&fin, 1);
		get_and_split_line(&fin, &tokens);
		this->certified_cost = convert<double>(tokens.at(4));

		skip_lines(&fin, 18 - num_parameters);
		for (int i = 0; i < num_observations; ++i) {
			get_and_split_line(&fin, &tokens);
			for (int j = 0; j < num_responses; ++j) {
				this->response(i, j) = convert<double>(tokens.at(j));
			}

			for (int j = 0; j < num_predictors; ++j) {
				this->predictor(i, j) =
					convert<double>(tokens.at(j + num_responses));
			}
		}
	}

	Eigen::MatrixXd predictor, response, initial_parameters, final_parameters;
	double certified_cost;
};

template<typename SolverClass>
SolverClass create_solver()
{
	return {};
}

template<>
NewtonSolver create_solver<NewtonSolver>()
{
	NewtonSolver solver;
	solver.maximum_iterations = 5000;
	solver.function_improvement_tolerance = 0;
	solver.argument_improvement_tolerance = 0;
	solver.gradient_tolerance = 1e-8;

	solver.line_search_rho = 0.6;

	solver.factorization_method = NewtonSolver::FactorizationMethod::MESCHACH;
	return solver;
}

const bool use_sym_ildl =
#ifdef USE_SYM_ILDL
	true;
#else
	false;
#endif

class NewtonSolverSYM_ILDL
	: public NewtonSolver
{
};

template<>
NewtonSolverSYM_ILDL create_solver<NewtonSolverSYM_ILDL>()
{
	NewtonSolverSYM_ILDL solver;
	static_cast<NewtonSolver&>(solver) = create_solver<NewtonSolver>();
	solver.factorization_method = NewtonSolver::FactorizationMethod::SYM_ILDL;
	return solver;
}

template<>
LBFGSSolver create_solver<LBFGSSolver>()
{
	LBFGSSolver solver;
	solver.maximum_iterations = 10000;
	solver.function_improvement_tolerance = 0;
	solver.argument_improvement_tolerance = 0;
	solver.gradient_tolerance = 1e-7;

	return solver;
}

template<>
NelderMeadSolver create_solver<NelderMeadSolver>()
{
	NelderMeadSolver solver;
	solver.maximum_iterations = 10000;
	solver.function_improvement_tolerance = 1e-14;
	solver.argument_improvement_tolerance = 1e-14;
	solver.gradient_tolerance = 1e-12;
	solver.area_tolerance = 1e-60;

	return solver;
}

template<typename SolverClass, typename Model, int num_variables>
void run_problem_main(const std::string& filename)
{
	NISTProblem problem(filename);
	REQUIRE(problem.response.cols() == 1);
	REQUIRE(problem.initial_parameters.cols() == num_variables);

	// Run a test for each starting point provicded by the data file.
	for (int start = 0; start < problem.initial_parameters.rows(); ++start) {
		INFO("Start " << start + 1 << " of " << problem.initial_parameters.rows());

		Eigen::VectorXd initial_parameters =
			problem.initial_parameters.row(start);
		REQUIRE(initial_parameters.size() == num_variables);

		Function function;
		function.add_variable(initial_parameters.data(), num_variables);

		for (int i = 0; i < problem.predictor.rows(); ++i) {
			double x = problem.predictor(i, 0);
			double y = problem.response(i, 0);
			auto term = std::make_shared<AutoDiffTerm<Model, num_variables>>(x, y);
			function.add_term(term, initial_parameters.data());
		}

		auto initial_cost = function.evaluate();

		auto solver = create_solver<SolverClass>();

		std::stringstream sout;
		solver.log_function = [&sout](const std::string& s) { sout << s << std::endl; };

		SolverResults results;
		solver.solve(function, &results);

		auto final_cost = function.evaluate();

		// Print the solver results to the log stringstream.
		INFO(sout.str());
		INFO(results);
		for (int i = 0; i < num_variables; ++i) {
			INFO("b[" << i << "] = " << initial_parameters[i]);
		}

		const double optimum = problem.certified_cost;
		int num_matching_digits = static_cast<int>(
			-std::log10(fabs(function.evaluate() - optimum) / optimum));

		INFO("Number of matching digits: " << num_matching_digits);

		// Compute log_relative_error with code from Ceres Solver.
		//
		// Compute the LRE by comparing each component of the solution
		// with the ground truth, and taking the minimum.
		auto final_parameters = problem.final_parameters;
		const double kMaxNumSignificantDigits = 11;
		double log_relative_error = kMaxNumSignificantDigits + 1;
		for (int i = 0; i < num_variables; ++i) {
			const double tmp_lre =
				-std::log10(std::fabs(final_parameters(i) - initial_parameters(i)) /
							std::fabs(final_parameters(i)));
			// The maximum LRE is capped at 11 - the precision at which the
			// ground truth is known.
			//
			// The minimum LRE is capped at 0 - no digits match between the
			// computed solution and the ground truth.
			log_relative_error =
			std::min(log_relative_error,
					 std::max(0.0, std::min(kMaxNumSignificantDigits, tmp_lre)));
		}

		output_file << typeid(Model).name() << " "
		            << typeid(SolverClass).name() << " "
		            << "start: " << start + 1 << " "
					<< (log_relative_error < 4 ? "FAILURE" : "SUCCESS") << " "
		            << "LRE: " << log_relative_error << " "
		            << "Initial cost: " << initial_cost << " "
		            << "Final cost: " << final_cost << " "
		            << "Certified cost: " << problem.certified_cost
					<< std::endl;

		// If the optimum was reached, everything is OK.
		if (num_matching_digits >= 4) {
			CHECK(num_matching_digits >= 4);  // To log the test.
			continue;
		}

		// Otherwise, reaching a stationary point is enough.
		CHECK(results.exit_condition ==
		      SolverResults::GRADIENT_TOLERANCE);

		// But for Nelder-Mead, a small area is not equivalent to a
		// stationary point.
		if (typeid(SolverClass) == typeid(NelderMeadSolver)) {
			CHECK(num_matching_digits >= 4);
		}
	}
}

#define NIST_TEST_START(Problem)         \
struct Problem                           \
{                                        \
	double x_param, y_param;             \
	Problem(double x, double y)          \
	{                                    \
		this->x_param = x;               \
		this->y_param = y;               \
	}                                    \
	template<typename R>                 \
	R operator()(const R* const b) const \
	{                                    \
		const R x(x_param);              \
		const R y(y_param);              \
		R d = y - (                      \

#define NIST_TEST_END(Category, Problem, n) \
		);                               \
		return d*d;                      \
	}                                    \
};                                       \
TEST_CASE(#Category "/" #Problem, "")    \
{                                        \
	SECTION("Newton") {					 \
		run_problem_main<NewtonSolver, Problem, n>(    \
			"nist/" #Problem ".dat");    \
	}                                    \
	SECTION("NewtonSolverSYM_ILDL") {    \
		if (use_sym_ildl) {              \
			run_problem_main<NewtonSolverSYM_ILDL, Problem, n>(    \
			"nist/" #Problem ".dat");    \
		}                                \
	}                                    \
	SECTION("LBFGS") {                   \
		run_problem_main<LBFGSSolver, Problem, n>(    \
			"nist/" #Problem ".dat");    \
	}                                    \
}

const double kPi = 3.141592653589793238462643383279;

NIST_TEST_START(Bennett5)
	b[0] * pow(b[1] + x, R(-1.0) / b[2])
NIST_TEST_END(Hard, Bennett5, 3)

NIST_TEST_START(BoxBOD)
  b[0] * (R(1.0) - exp(-b[1] * x))
NIST_TEST_END(Hard, BoxBOD, 2)

NIST_TEST_START(Chwirut1)
  exp(-b[0] * x) / (b[1] + b[2] * x)
NIST_TEST_END(Easy, Chwirut1, 3)

NIST_TEST_START(Chwirut2)
  exp(-b[0] * x) / (b[1] + b[2] * x)
NIST_TEST_END(Easy, Chwirut2, 3)

NIST_TEST_START(DanWood)
  b[0] * pow(x, b[1])
NIST_TEST_END(Easy, DanWood, 2)

NIST_TEST_START(Gauss1)
  b[0] * exp(-b[1] * x) +
  b[2] * exp(-pow((x - b[3])/b[4], 2)) +
  b[5] * exp(-pow((x - b[6])/b[7],2))
NIST_TEST_END(Easy, Gauss1, 8)

NIST_TEST_START(Gauss2)
  b[0] * exp(-b[1] * x) +
  b[2] * exp(-pow((x - b[3])/b[4], 2)) +
  b[5] * exp(-pow((x - b[6])/b[7],2))
NIST_TEST_END(Medium, Gauss2, 8)

NIST_TEST_START(Gauss3)
  b[0] * exp(-b[1] * x) +
  b[2] * exp(-pow((x - b[3])/b[4], 2)) +
  b[5] * exp(-pow((x - b[6])/b[7],2))
NIST_TEST_END(Medium, Gauss3, 8)

NIST_TEST_START(Lanczos1)
  b[0] * exp(-b[1] * x) + b[2] * exp(-b[3] * x) + b[4] * exp(-b[5] * x)
NIST_TEST_END(Medium, Lanczos1, 6)

NIST_TEST_START(Lanczos2)
  b[0] * exp(-b[1] * x) + b[2] * exp(-b[3] * x) + b[4] * exp(-b[5] * x)
NIST_TEST_END(Medium, Lanczos2, 6)

NIST_TEST_START(Hahn1)
  (b[0] + b[1] * x + b[2] * x * x + b[3] * x * x * x) /
  (R(1.0) + b[4] * x + b[5] * x * x + b[6] * x * x * x)
NIST_TEST_END(Medium, Hahn1, 7)

NIST_TEST_START(Kirby2)
  (b[0] + b[1] * x + b[2] * x * x) /
  (R(1.0) + b[3] * x + b[4] * x * x)
NIST_TEST_END(Medium, Kirby2, 5)

NIST_TEST_START(MGH09)
  b[0] * (x * x + x * b[1]) / (x * x + x * b[2] + b[3])
NIST_TEST_END(Hard, MGH09, 4)

NIST_TEST_START(MGH10)
  b[0] * exp(b[1] / (x + b[2]))
NIST_TEST_END(Hard, MGH10, 3)

NIST_TEST_START(MGH17)
  b[0] + b[1] * exp(-x * b[3]) + b[2] * exp(-x * b[4])
NIST_TEST_END(Medium, MGH17, 5)

NIST_TEST_START(Misra1a)
  b[0] * (R(1.0) - exp(-b[1] * x))
NIST_TEST_END(Easy, Misra1a, 2)

NIST_TEST_START(Misra1b)
  b[0] * (R(1.0) - R(1.0)/ ((R(1.0) + b[1] * x / 2.0) * (R(1.0) + b[1] * x / 2.0)))
NIST_TEST_END(Easy, Misra1b, 2)

NIST_TEST_START(Misra1c)
  b[0] * (R(1.0) - pow(R(1.0) + R(2.0) * b[1] * x, -0.5))
NIST_TEST_END(Medium, Misra1c, 2)

NIST_TEST_START(Misra1d)
  b[0] * b[1] * x / (R(1.0) + b[1] * x)
NIST_TEST_END(Medium, Misra1d, 2)

NIST_TEST_START(Roszman1)
  b[0] - b[1] * x - atan(b[2] / (x - b[3]))/R(kPi)
NIST_TEST_END(Medium, Roszman1, 4)

NIST_TEST_START(Rat42)
  b[0] / (R(1.0) + exp(b[1] - b[2] * x))
NIST_TEST_END(Hard, Rat42, 3)

NIST_TEST_START(Rat43)
  b[0] / pow(R(1.0) + exp(b[1] - b[2] * x), R(1.0) / b[3])
NIST_TEST_END(Hard, Rat43, 4)

NIST_TEST_START(Thurber)
  (b[0] + b[1] * x + b[2] * x * x  + b[3] * x * x * x) /
  (R(1.0) + b[4] * x + b[5] * x * x + b[6] * x * x * x)
NIST_TEST_END(Hard, Thurber, 7)

NIST_TEST_START(ENSO)
  b[0] + b[1] * cos(R(2.0 * kPi) * x / R(12.0)) +
         b[2] * sin(R(2.0 * kPi) * x / R(12.0)) +
         b[4] * cos(R(2.0 * kPi) * x / b[3]) +
         b[5] * sin(R(2.0 * kPi) * x / b[3]) +
         b[7] * cos(R(2.0 * kPi) * x / b[6]) +
         b[8] * sin(R(2.0 * kPi) * x / b[6])
NIST_TEST_END(Medium, ENSO, 9)

NIST_TEST_START(Eckerle4)
  b[0] / b[1] * exp(R(-0.5) * pow((x - b[2])/b[1], 2))
NIST_TEST_END(Hard, Eckerle4, 3)

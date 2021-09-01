// Petter Strandmark 2012–2013.

#include <stdexcept>

#include <spii/solver.h>

namespace spii {

std::ostream& operator<<(std::ostream& out, const SolverResults& results)
{
	out << "----------------------------------------------\n";
	out << "Exit condition            : ";
	#define EXIT_ENUM_IF(val) if (results.exit_condition == SolverResults::val) out << #val << '\n';
	EXIT_ENUM_IF(GRADIENT_TOLERANCE);
	EXIT_ENUM_IF(FUNCTION_TOLERANCE);
	EXIT_ENUM_IF(ARGUMENT_TOLERANCE);
	EXIT_ENUM_IF(NO_CONVERGENCE);
	EXIT_ENUM_IF(FUNCTION_NAN);
	EXIT_ENUM_IF(FUNCTION_INFINITY);
	EXIT_ENUM_IF(USER_ABORT);
	EXIT_ENUM_IF(INTERNAL_ERROR);
	EXIT_ENUM_IF(NA);
	out << "----------------------------------------------\n";
	out << "Startup time              : " << results.startup_time << '\n';
	out << "Function evaluation time  : " << results.function_evaluation_time << '\n';
	out << "Stopping criteria time    : " << results.stopping_criteria_time << '\n';
	out << "Matrix factorization time : " << results.matrix_factorization_time << '\n';
	out << "L-BFGS update time        : " << results.lbfgs_update_time << '\n';
	out << "Linear solver time        : " << results.linear_solver_time << '\n';
	out << "Backtracking time         : " << results.backtracking_time << '\n';
	out << "Log time                  : " << results.log_time << '\n';
	out << "Total time (without log)  : " << results.total_time - results.log_time << '\n';
	out << "----------------------------------------------\n";
	return out;
}

Solver::Solver()
{
	this->log_function = []
	                     (const std::string& msg)
						 {
							#ifdef EMSCRIPTEN
								std::cout
							#else
								std::cerr
							#endif
							<< msg << std::endl;
						 };

	#ifdef _MSC_VER
		#if _MSC_VER < 1900
			_set_output_format(_TWO_DIGIT_EXPONENT);
		#endif
	#endif
}


Solver::~Solver()
{ }


}  // namespace spii


// Petter Strandmark 2012.

#include <spii/solver.h>

namespace spii {

bool Solver::check_exit_conditions(const double fval,
                                   const double fprev,
                                   const double normg,
                                   const double normg0,
                                   const double normx,
                                   const double normdx,
                                   const bool last_iteration_successful,
                                   CheckExitConditionsCache* cache,
                                   SolverResults* results) const
{
	if (fval != fval) {
		// NaN encountered.
		if (this->log_function) {
			this->log_function("f(x) is NaN.");
		}
		results->exit_condition = SolverResults::FUNCTION_NAN;
		return true;
	}

	if (normg / normg0 < this->gradient_tolerance) {
		// We have reached a small enough gradient. Terminate if it
		// does not seem to improve anymore.
		
		// Look at the recent history and find the largest gradient.
		auto max_normg = cache->normg_history[0];
		for (auto ng: cache->normg_history) {
			max_normg = std::max(max_normg, ng);
		}

		// If it does not seem to improve more or if it is really small.
		if (normg / max_normg >= 0.9 ||
			normg / normg0 < (this->gradient_tolerance * this->gradient_tolerance)) {

			results->exit_condition = SolverResults::GRADIENT_TOLERANCE;
			return true;
		}
	}

	cache->norm_g_history_pos = (cache->norm_g_history_pos + 1) % cache->amount_history_to_consider;
	cache->normg_history[cache->norm_g_history_pos] = normg;

	if (last_iteration_successful &&
	    std::fabs(fval - fprev) / (std::fabs(fval) + this->function_improvement_tolerance) <
	                                                 this->function_improvement_tolerance) {
		results->exit_condition = SolverResults::FUNCTION_TOLERANCE;
		return true;
	}

	if (last_iteration_successful &&
	    normdx / (normx + this->argument_improvement_tolerance) <
	                      this->argument_improvement_tolerance) {
		results->exit_condition = SolverResults::ARGUMENT_TOLERANCE;
		return true;
	}

	if (fval ==  std::numeric_limits<double>::infinity() ||
		fval == -std::numeric_limits<double>::infinity()) {
		// Infinity encountered.
		if (this->log_function) {
			this->log_function("f(x) is infinity.");
		}
		results->exit_condition = SolverResults::FUNCTION_INFINITY;
		return true;
	}
	return false;
}

}  // namespace spii

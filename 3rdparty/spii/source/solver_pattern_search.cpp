// Petter Strandmark 2012.

#include <cstdio>
#include <iostream>
#include <limits>
#include <stdexcept>

#include <Eigen/Dense>

#include <spii/spii.h>
#include <spii/solver.h>

namespace spii {

void PatternSolver::solve(const Function& function,
                          SolverResults* results) const
{
	double global_start_time = wall_time();

	// Dimension of problem.
	size_t n = function.get_number_of_scalars();

	if (n == 0) {
		results->exit_condition = SolverResults::FUNCTION_TOLERANCE;
		return;
	}

	// Current point, gradient and Hessian.
	double fval   = std::numeric_limits<double>::quiet_NaN();
	double fprev  = std::numeric_limits<double>::quiet_NaN();

	Eigen::VectorXd x;
	// Copy the user state to the current point.
	function.copy_user_to_global(&x);
	Eigen::VectorXd dx(n);

	// Size of the pattern.
	double pattern_size = 1.0;
	double x_inf = std::max(x.maxCoeff(), -x.minCoeff());
	if (x_inf > 1e-6) {
		pattern_size = x_inf /= 2.0;
	}
	double pattern_size0 = pattern_size;

	// Sufficient decrease.
	auto rho = [](double t) -> double { return 1e-4 * std::pow(t ,1.5); };


	//
	// START MAIN ITERATION
	//
	results->startup_time   += wall_time() - global_start_time;
	results->exit_condition = SolverResults::INTERNAL_ERROR;
	int iter = 0;
	while (true) {

		//
		// Search along all coordinate directions
		//
		double start_time = wall_time();

		if (iter == 0) {
			fval = function.evaluate(x);
		}

		bool success = false;
		for (size_t i = 0; i < n; ++i) {
			dx.setZero();
			//for d in {-1, 1}
			for (double d = -1; d <= 1; d += 2) {
				// Search along this coordinate axis in
				// direction d.
				dx[i] = d * pattern_size;
				// Evaluate function in new point.
				double fval_new = function.evaluate(x + dx);

				// If we have sufficient decrease.
				if (fval_new < fval - rho(pattern_size)) {
					success = true;
					x = x + dx;
					fval = fval_new;
					break;
				}
			}

			if (success) {
				break;
			}
		}

		// If no point was found, decrease pattern size.
		if (! success) {
			pattern_size /= 2.0;
		}

		results->function_evaluation_time += wall_time() - start_time;

		//
		// Test stopping criteriea
		//
		start_time = wall_time();
		if (pattern_size / pattern_size0 < this->area_tolerance) {
			results->exit_condition = SolverResults::GRADIENT_TOLERANCE;
			break;
		}
		if (success &&
			std::fabs(fval - fprev) / (std::fabs(fval) + this->function_improvement_tolerance) <
			                                             this->function_improvement_tolerance) {
			results->exit_condition = SolverResults::FUNCTION_TOLERANCE;
			break;
		}
		if (iter >= this->maximum_iterations) {
			results->exit_condition = SolverResults::NO_CONVERGENCE;
			break;
		}
		results->stopping_criteria_time += wall_time() - start_time;

		//
		// Log the results of this iteration.
		//
		start_time = wall_time();

		int log_interval = 1;
		if (iter > 30) {
			log_interval = 10;
		}
		if (iter > 200) {
			log_interval = 100;
		}
		if (iter > 2000) {
			log_interval = 1000;
		}
		if (iter > 20000) {
			log_interval = 10000;
		}
		if (iter > 200000) {
			log_interval = 100000;
		}
		if (this->log_function && iter % log_interval == 0) {
			char str[1024];
				if (iter == 0) {
					this->log_function("Itr         f       deltaf   gamma   success");
				}
				std::sprintf(str, "%7d %+10.3e %9.3e %9.3e %s",
					iter, fval, std::fabs(fval - fprev), pattern_size, success ? "yes" : "no");
			this->log_function(str);
		}
		results->log_time += wall_time() - start_time;

		fprev = fval;
		iter++;
	}

	function.copy_global_to_user(x);
	results->total_time += wall_time() - global_start_time;

	if (this->log_function) {
		char str[1024];
		std::sprintf(str, "    end %+10.3e %9.3e %9.3e", fval, std::fabs(fval - fprev), pattern_size);
		this->log_function(str);
	}
}

}  // namespace spii
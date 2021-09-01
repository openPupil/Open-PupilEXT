// Petter Strandmark 2012.

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

#include <Eigen/Dense>

#include <spii/spii.h>
#include <spii/solver.h>

namespace spii {

void LBFGSSolver::solve(const Function& function,
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
	double normg0 = std::numeric_limits<double>::quiet_NaN();
	double normg  = std::numeric_limits<double>::quiet_NaN();
	double normdx = std::numeric_limits<double>::quiet_NaN();

	Eigen::VectorXd x, g;

	// Copy the user state to the current point.
	function.copy_user_to_global(&x);
	Eigen::VectorXd x2(n);

	// L-BFGS history.
	std::vector<Eigen::VectorXd>  s_data(this->lbfgs_history_size),
	                              y_data(this->lbfgs_history_size);
	std::vector<Eigen::VectorXd*> s(this->lbfgs_history_size),
	                              y(this->lbfgs_history_size);
	for (int h = 0; h < this->lbfgs_history_size; ++h) {
		s_data[h].resize(function.get_number_of_scalars());
		s_data[h].setZero();
		y_data[h].resize(function.get_number_of_scalars());
		y_data[h].setZero();
		s[h] = &s_data[h];
		y[h] = &y_data[h];
	}

	Eigen::VectorXd rho(this->lbfgs_history_size);
	rho.setZero();

	Eigen::VectorXd alpha(this->lbfgs_history_size);
	alpha.setZero();
	Eigen::VectorXd q(n);
	Eigen::VectorXd r(n);

	// Needed from the previous iteration.
	Eigen::VectorXd x_prev(n), s_tmp(n), y_tmp(n);

	CheckExitConditionsCache exit_condition_cache;

	//
	// START MAIN ITERATION
	//
	results->startup_time   += wall_time() - global_start_time;
	results->exit_condition = SolverResults::INTERNAL_ERROR;
	int iter = 0;
	bool last_iteration_successful = true;
	int number_of_line_search_failures = 0;
	int number_of_restarts = 0;
	while (true) {

		//
		// Evaluate function and derivatives.
		//
		double start_time = wall_time();
		// y[0] should contain the difference between the gradient
		// in this iteration and the gradient from the previous.
		// Therefore, update y before and after evaluating the
		// function.
		if (iter > 0) {
			y_tmp = -g;
		}
		fval = function.evaluate(x, &g);

		normg = std::max(g.maxCoeff(), -g.minCoeff());
		if (iter == 0) {
			normg0 = normg;
		}
		results->function_evaluation_time += wall_time() - start_time;

		//
		// Update history
		//
		start_time = wall_time();

		if (iter > 0 && last_iteration_successful) {
			s_tmp = x - x_prev;
			y_tmp += g;

			double sTy = s_tmp.dot(y_tmp);
			if (sTy > 1e-16) {
				// Shift all pointers one step back, discarding the oldest one.
				Eigen::VectorXd* sh = s[this->lbfgs_history_size - 1];
				Eigen::VectorXd* yh = y[this->lbfgs_history_size - 1];
				for (int h = this->lbfgs_history_size - 1; h >= 1; --h) {
					s[h]   = s[h - 1];
					y[h]   = y[h - 1];
					rho[h] = rho[h - 1];
				}
				// Reuse the storage of the discarded data for the new data.
				s[0] = sh;
				y[0] = yh;

				*y[0] = y_tmp;
				*s[0] = s_tmp;
				rho[0] = 1.0 / sTy;
			}
		}

		results->lbfgs_update_time += wall_time() - start_time;

		//
		// Test stopping criteriea
		//
		start_time = wall_time();
		if (iter > 1 && this->check_exit_conditions(fval, fprev, normg,
		                                            normg0, x.norm(), normdx,
		                                            last_iteration_successful, 
		                                            &exit_condition_cache, results)) {
			break;
		}
		if (iter >= this->maximum_iterations) {
			results->exit_condition = SolverResults::NO_CONVERGENCE;
			break;
		}

		if (this->callback_function) {
			CallbackInformation information;
			information.objective_value = fval;
			information.x = &x;
			information.g = &g;

			if (!callback_function(information)) {
				results->exit_condition = SolverResults::USER_ABORT;
				break;
			}
		}

		results->stopping_criteria_time += wall_time() - start_time;

		//
		// Compute search direction via L-BGFS two-loop recursion.
		//
		start_time = wall_time();
		bool should_restart = false;

		double H0 = 1.0;
		if (iter > 0) {
			// If the gradient is identical two iterations in a row,
			// y will be the zero vector and H0 will be NaN. In this
			// case the line search will fail and L-BFGS will be restarted
			// with a steepest descent step.
			H0 = s[0]->dot(*y[0]) / y[0]->dot(*y[0]);

			// If isinf(H0) || isnan(H0)
			if (H0 ==  std::numeric_limits<double>::infinity() ||
			    H0 == -std::numeric_limits<double>::infinity() ||
			    H0 != H0) {
				should_restart = true;
			}
		}

		q = -g;

		for (int h = 0; h < this->lbfgs_history_size; ++h) {
			alpha[h] = rho[h] * s[h]->dot(q);
			q = q - alpha[h] * (*y[h]);
		}

		r = H0 * q;

		for (int h = this->lbfgs_history_size - 1; h >= 0; --h) {
			double beta = rho[h] * y[h]->dot(r);
			r = r + (*s[h]) * (alpha[h] - beta);
		}

		// If the function improves very little, the approximated Hessian
		// might be very bad. If this is the case, it is better to discard
		// the history once in a while. This allows the solver to correctly
		// solve some badly scaled problems.
		double restart_test = std::fabs(fval - fprev) /
		                      (std::fabs(fval) + std::fabs(fprev));
		if (iter > 0 && iter % 100 == 0 && restart_test
		                                   < this->lbfgs_restart_tolerance) {
			should_restart = true;
		}
		if (! last_iteration_successful) {
			should_restart = true;
		}

		if (should_restart) {
			if (this->log_function) {
				char str[1024];
				if (number_of_restarts <= 10) {
					std::sprintf(str, "Restarting: fval = %.3e, deltaf = %.3e, max|g_i| = %.3e, test = %.3e",
								 fval, std::fabs(fval - fprev), normg, restart_test);
					this->log_function(str);
				}
				if (number_of_restarts == 10) {
					this->log_function("NOTE: No more restarts will be reported.");
				}
				number_of_restarts++;
			}
			r = -g;
			for (int h = 0; h < this->lbfgs_history_size; ++h) {
				(*s[h]).setZero();
				(*y[h]).setZero();
			}
			rho.setZero();
			alpha.setZero();
			// H0 is not used, but its value will be printed.
			H0 = std::numeric_limits<double>::quiet_NaN();
		}

		results->lbfgs_update_time += wall_time() - start_time;

		//
		// Perform line search.
		//
		start_time = wall_time();
		double start_alpha = 1.0;
		// In the first iteration, start with a much smaller step
		// length. (heuristic used by e.g. minFunc)
		if (iter == 0) {
			double sumabsg = 0.0;
			for (size_t i = 0; i < n; ++i) {
				sumabsg += std::fabs(g[i]);
			}
			start_alpha = std::min(1.0, 1.0 / sumabsg);
		}
		double alpha_step = this->perform_linesearch(function, x, fval, g,
		                                             r, &x2, start_alpha);

		if (alpha_step <= 0) {
			if (this->log_function) {
				this->log_function("Line search failed.");
				char str[1024];
				std::sprintf(str, "%4d %+.3e %9.3e %.3e %.3e %.3e %.3e",
					iter, fval, std::fabs(fval - fprev), normg, alpha_step, H0, rho[0]);
				this->log_function(str);
			}
			if (! last_iteration_successful || number_of_line_search_failures++ > 10) {
				// This happens quite seldom. Every time it has happened, the function
				// was actually converged to a solution.
				results->exit_condition = SolverResults::GRADIENT_TOLERANCE;
				break;
			}

			last_iteration_successful = false;
		}
		else {
			// Record length of this step.
			normdx = alpha_step * r.norm();
			// Compute new point.
			x_prev = x;
			x = x + alpha_step * r;

			last_iteration_successful = true;
		}

		results->backtracking_time += wall_time() - start_time;

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
		if (this->log_function && iter % log_interval == 0) {
			if (iter == 0) {
				this->log_function("Itr       f       deltaf   max|g_i|   alpha      H0       rho");
			}

			this->log_function(
				to_string(
					std::setw(4), iter, " ",
					std::setw(10), std::setprecision(3), std::scientific, std::showpos, fval, std::noshowpos, " ",
					std::setw(9),  std::setprecision(3), std::scientific, std::fabs(fval - fprev), " ",
					std::setw(9),  std::setprecision(3), std::setprecision(3), std::scientific, normg, " ",
					std::setw(9),  std::setprecision(3), std::scientific, alpha_step, " ",
					std::setw(9),  std::setprecision(3), std::scientific, H0, " ",
					std::setw(9),  std::setprecision(3), std::scientific, rho[0]
				)
			);
		}
		results->log_time += wall_time() - start_time;

		fprev = fval;
		iter++;
	}

	function.copy_global_to_user(x);
	results->total_time += wall_time() - global_start_time;

	if (this->log_function) {
		char str[1024];
		std::sprintf(str, " end %+.3e           %.3e", fval, normg);
		this->log_function(str);
	}
}

}  // namespace spii
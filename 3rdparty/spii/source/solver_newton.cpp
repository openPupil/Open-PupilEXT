// Petter Strandmark 2012.

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>

// GNU 4.8.1 define _X on Cygwin.
// This breaks Eigen.
// http://eigen.tuxfamily.org/bz/process_bug.cgi
#ifdef _X
#undef _X
#endif
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <Eigen/Sparse>

#include <spii/spii.h>
#include <spii/solver.h>

namespace spii {

void NewtonSolver::solve(const Function& function,
                         SolverResults* results) const
{
	double global_start_time = wall_time();

	// Random number engine for random pertubation.
	std::mt19937 prng(unsigned(1));
	std::uniform_real_distribution<double> uniform11(-1.0, 1.0);
	auto rand11 = std::bind(uniform11, prng);

	// Dimension of problem.
	size_t n = function.get_number_of_scalars();

	if (n == 0) {
		results->exit_condition = SolverResults::FUNCTION_TOLERANCE;
		return;
	}

	// Determine whether to use sparse representation
	// and matrix factorization.
	bool use_sparsity;
	if (this->sparsity_mode == SparsityMode::DENSE) {
		use_sparsity = false;
	}
	else if (this->sparsity_mode == SparsityMode::SPARSE) {
		use_sparsity = true;
	}
	else {
		if (n <= 50) {
			use_sparsity = false;
		}
		else {
			use_sparsity = true;
		}
	}

	auto factorization_method = this->factorization_method;
	if (use_sparsity && this->factorization_method == FactorizationMethod::MESCHACH) {
		if (this->log_function) {
			this->log_function("Can not use the Meschach library for sparse problems. Switching to iterative factorization.");
		}
		factorization_method = FactorizationMethod::ITERATIVE;
	}

	// Current point, gradient and Hessian.
	double fval   = std::numeric_limits<double>::quiet_NaN();;
	double fprev  = std::numeric_limits<double>::quiet_NaN();
	double normg0 = std::numeric_limits<double>::quiet_NaN();
	double normg  = std::numeric_limits<double>::quiet_NaN();
	double normdx = std::numeric_limits<double>::quiet_NaN();

	Eigen::VectorXd x, g;
	Eigen::MatrixXd H;
	Eigen::SparseMatrix<double> sparse_H;
	if (use_sparsity) {
		// Create sparsity pattern for H.
		function.create_sparse_hessian(&sparse_H);
		if (this->log_function) {
			double nnz = double(sparse_H.nonZeros()) / double(n * n);
			char str[1024];
			std::sprintf(str, "H is %dx%d with %d (%.5f%%) non-zeroes.",
				sparse_H.rows(), sparse_H.cols(), sparse_H.nonZeros(), 100.0 * nnz);
			this->log_function(str);
		}
	}

	// Copy the user state to the current point.
	function.copy_user_to_global(&x);
	Eigen::VectorXd x2(n);

	// p will store the search direction.
	Eigen::VectorXd p(function.get_number_of_scalars());

	// Dense and sparse Cholesky factorizers.
	typedef Eigen::LLT<Eigen::MatrixXd> LLT;
	typedef Eigen::SimplicialLLT<Eigen::SparseMatrix<double> > SparseLLT;
	std::unique_ptr<LLT> factorization;
	std::unique_ptr<SparseLLT> sparse_factorization;
	if (!use_sparsity) {
		factorization.reset(new LLT(n));
	}
	else {
		sparse_factorization.reset(new SparseLLT);
		// The sparsity pattern of H is always the same. Therefore, it is enough
		// to analyze it once.
		sparse_factorization->analyzePattern(sparse_H);
	}

	FactorizationCache factorization_cache((int)n);
	CheckExitConditionsCache exit_condition_cache;

	//
	// START MAIN ITERATION
	//
	results->startup_time   += wall_time() - global_start_time;
	results->exit_condition = SolverResults::INTERNAL_ERROR;
	int iter = 0;
	while (true) {

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

		//
		// Evaluate function and derivatives.
		//
		double start_time = wall_time();
		if (use_sparsity) {
			fval = function.evaluate(x, &g, &sparse_H);
		}
		else {
			fval = function.evaluate(x, &g, &H);
		}

		normg = std::max(g.maxCoeff(), -g.minCoeff());
		if (iter == 0) {
			normg0 = normg;
		}

		// Check for NaN.
		if (normg != normg) {
			results->exit_condition = SolverResults::FUNCTION_NAN;
			break;
		}

		results->function_evaluation_time += wall_time() - start_time;

		//
		// Test stopping criteriea
		//
		start_time = wall_time();
		if (this->check_exit_conditions(fval, fprev, normg,
			                            normg0, x.norm(), normdx,
			                            true, &exit_condition_cache, results)) {
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
			if (use_sparsity) {
				information.H_sparse = &sparse_H;
			}
			else {
				information.H_dense = &H;
			}

			if (!callback_function(information)) {
				results->exit_condition = SolverResults::USER_ABORT;
				break;
			}
		}
		results->stopping_criteria_time += wall_time() - start_time;


		int factorizations = 0;
		double tau = 0;
		double mindiag = 0;
		Eigen::VectorXd dH;
		if (use_sparsity) {
			dH = sparse_H.diagonal();
		}
		else {
			dH = H.diagonal();
		}
		mindiag = dH.minCoeff();

		if (factorization_method == FactorizationMethod::ITERATIVE) {
			//
			// Attempt repeated Cholesky factorization until the Hessian
			// becomes positive semidefinite.
			//
			//start_time = wall_time();
			double beta = 1.0;

			if (mindiag > 0) {
				tau = 0;
			}
			else {
				tau = -mindiag + beta;
			}
			while (true) {
				// Add tau*I to the Hessian.
				if (tau > 0) {
					for (size_t i = 0; i < n; ++i) {
						if (use_sparsity) {
							int ii = static_cast<int>(i);
							sparse_H.coeffRef(ii, ii) = dH(i) + tau;
						}
						else {
							H(i, i) = dH(i) + tau;
						}
					}
				}
				// Attempt Cholesky factorization.
				bool success;
				if (use_sparsity) {
					sparse_factorization->factorize(sparse_H);
					success = sparse_factorization->info() == Eigen::Success;
				}
				else {
					factorization->compute(H);
					success = factorization->info() == Eigen::Success;
				}
				factorizations++;
				// Check for success.
				if (success) {
					break;
				}
				tau = std::max(2*tau, beta);

				spii_assert(factorizations <= 100,
				            "Solver::solve: factorization failed.");
			}
		

			results->matrix_factorization_time += wall_time() - start_time;

			//
			// Solve linear system to obtain search direction.
			//
			start_time = wall_time();

			if (use_sparsity) {
				p = sparse_factorization->solve(-g);
			}
			else {
				p = factorization->solve(-g);
			}

			results->linear_solver_time += wall_time() - start_time;
		}
		else if (factorization_method == FactorizationMethod::MESCHACH) {
			spii_assert(!use_sparsity);

			// Performs a BKP block diagonal factorization, modifies it, and
			// solvers the linear system.
			this->BKP_dense(H, g, factorization_cache, &p, results);
			factorizations = 1;
		}
		else if (factorization_method == FactorizationMethod::SYM_ILDL) {
			factorizations = 1;
			if (use_sparsity) {
				this->BKP_sym_ildl(sparse_H, g, &p, results);
			}
			else {
				this->BKP_sym_ildl(H, g, &p, results);
			}
		}
		else {
			throw std::runtime_error("Unknown factorization method.");
		}

		//
		// Perform line search.
		//
		start_time = wall_time();
		double start_alpha = 1.0;
		double alpha = this->perform_linesearch(function, x, fval, g, p, &x2,
		                                        start_alpha);

		if (alpha <= 1e-15) {

			// Attempt a simple steepest descent instead.
			p = -g;
			alpha = this->perform_linesearch(function, x, fval, g, p, &x2,
			                                 1.0);
			if (alpha <= 0) {

				if (this->log_function) {
					this->log_function("Steepest descent step failed. Numerical problems?");
				}

				// This happens in really rare cases with numerical problems or
				// incorrectly defined objective functions. In the latter case,
				// there is not much to do. In the former case, randomly perturbing
				// x has been effective.
				for (size_t i = 0; i < n; ++i) {
					x[i] = x[i] + 1e-6 * rand11() * x[i];
				}
				
				alpha = 0;
			}
		}

		// Record length of this step.
		normdx = alpha * p.norm();
		// Update current point.
		x = x + alpha * p;

		results->backtracking_time += wall_time() - start_time;

		//
		// Log the results of this iteration.
		//
		start_time = wall_time();

		if (this->log_function && iter % log_interval == 0) {
			if (use_sparsity) {
				if (iter == 0) {
					this->log_function("Itr        f        max|g_i|   alpha    fac    tau   min(H_ii)");
				}
				this->log_function(
					to_string(std::setw(4), iter) + " " +
					to_string(std::scientific, std::showpos, std::setprecision(6), std::setw(10), fval) + " " +
					to_string(std::scientific, std::setprecision(3), std::setw(9), normg) + " " +
					to_string(std::scientific, std::setprecision(3), std::setw(9), alpha) + " " +
					to_string(std::setw(3), factorizations) + "   " +
					to_string(std::scientific, std::setprecision(1), tau) + " " +
					to_string(std::scientific, std::showpos, std::setprecision(2), mindiag)
					);
			}
			else {
				double detH = H.determinant();
				double normH = H.norm();

				if (iter == 0) {
					this->log_function("Itr        f        max|g_i|   ||H||     det(H)    min(H_ii)  alpha    fac");
				}

				this->log_function(
					to_string(std::setw(4), iter) + " " +
					to_string(std::scientific, std::showpos, std::setprecision(6), std::setw(10), fval) + " " +
					to_string(std::scientific, std::setprecision(3), std::setw(9), normg) + " " +
					to_string(std::scientific, std::setprecision(3), std::setw(9), normH) + " " +
					to_string(std::scientific, std::showpos, std::setprecision(3), std::setw(10), detH) + " " +
					to_string(std::scientific, std::showpos, std::setprecision(2), mindiag) + " " +
					to_string(std::scientific, std::setprecision(3), std::setw(9), alpha) + " " +
					to_string(std::setw(3), factorizations)
					);
			}
		}
		results->log_time += wall_time() - start_time;

		fprev = fval;
		iter++;
	}

	function.copy_global_to_user(x);
	results->total_time += wall_time() - global_start_time;

	if (this->log_function) {
		char str[1024];
		std::sprintf(str, " end %+10.6e %.3e", fval, normg);
		this->log_function(str);
	}
}

}  // namespace spii

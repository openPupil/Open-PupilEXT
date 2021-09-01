// Petter Strandmark 2013.

#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>

// GNU 4.8.1 define _X on Cygwin.
// This breaks Eigen.
// http://eigen.tuxfamily.org/bz/show_bug.cgi?id=658
#ifdef _X
#undef _X
#endif
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <Eigen/Sparse>

#include <spii/spii.h>
#include <spii/solver.h>

#ifndef USE_SYM_ILDL

void spii::Solver::BKP_sym_ildl(const Eigen::MatrixXd& Hinput,
                                const Eigen::VectorXd& g,
                                Eigen::VectorXd* p,
                                SolverResults* results) const
{
	throw std::runtime_error("sym-ildl is not available.");
}

void spii::Solver::BKP_sym_ildl(const Eigen::SparseMatrix<double>& Hinput,
                                const Eigen::VectorXd& g,
                                Eigen::VectorXd* p,
                                SolverResults* results) const
{
	throw std::runtime_error("sym-ildl is not available.");
}

#else

#include <lilc_matrix.h>

#include <spii/sym-ildl-conversions.h>

namespace spii {

namespace{
void modify_block_diagonal_matrix(block_diag_matrix<double>* B)
{
	using namespace Eigen;

	auto n = B->n_rows();
	spii_assert(B->n_cols() == n);

	//
	// Modify the block diagonalization.
	//
	const double delta = 1e-12;

	VectorXd tau(n);
	VectorXd lambda(n);

	SelfAdjointEigenSolver<MatrixXd> eigensolver;

	bool onebyone;
	for (int i = 0; i < n; i = (onebyone ? i+1 : i+2) ) {
		onebyone = (i == n-1 || B->block_size(i) == 1);

		if ( onebyone ) {
			auto& Bii = (*B)[i];

			double lambda = Bii;
			double tau;
			if (lambda >= delta) {
				tau = 0;
			}
			else {
				tau = delta - (1.0 + delta) * lambda;
			}
			//Q(i, i) = 1;
			Bii = Bii + tau;
		}
		else {
			Matrix2d Bblock;
			Bblock(0, 0) = (*B)[i];
			Bblock(0, 1) = B->off_diagonal(i);
			Bblock(1, 0) = B->off_diagonal(i);
			Bblock(1, 1) = (*B)[i+1];
			spii_assert(Bblock(1, 0) == Bblock(0, 1));

			eigensolver.compute(Bblock);
			Vector2d lambda;
			lambda(0) = eigensolver.eigenvalues()(0);
			lambda(1) = eigensolver.eigenvalues()(1);

			Vector2d tau;
			for (int k = 0; k < 2; ++k) {
				if (lambda(k) >= delta) {
					tau(k) = 0;
				}
				else {
					tau(k) = delta - (1.0 + delta) * lambda(k);
				}
			}

			Matrix2d Qblock = eigensolver.eigenvectors();
			Bblock = Bblock + Qblock * tau.asDiagonal() * Qblock.transpose();
			(*B)[i]            = Bblock(0, 0);
			B->off_diagonal(i) = Bblock(0, 1);
			(*B)[i+1]          = Bblock(1, 1);
		}
	}
}
} // anon. namespace


template<typename MatrixType>
void BKP_sym_ildl_generic(const MatrixType& Hinput,
                          const Eigen::VectorXd& g,
                          Eigen::VectorXd* p,
                          SolverResults* results)
{
	using namespace std;
	using namespace Eigen;
	double start_time = wall_time();

	//
	// Create sym-ildl matrix.
	//
	lilc_matrix<double> Hlilc;
	eigen_to_lilc(Hinput, &Hlilc);

	//
	// Factorize the matrix.
	//
	lilc_matrix<double> L;	          // The lower triangular factor of A.
	vector<int> perm;	                  // A permutation vector containing all permutations on A.
	perm.reserve(Hlilc.n_cols());
	block_diag_matrix<double> B; // The diagonal factor of A.

	Hlilc.sym_amd(perm);
	Hlilc.sym_perm(perm);

	const double fill_factor = 1.0;
	const double tol         = 1e-12;
	const double pp_tol      = 1.0; // For full Bunch-Kaufman.
	Hlilc.ildl(L, B, perm, fill_factor, tol, pp_tol);

	// Convert back to Eigen matrices.
	MyPermutation P(perm);
	auto S = diag_to_eigen(Hlilc.S);

	//
	// Modify the block diagonalization.
	//
	modify_block_diagonal_matrix(&B);

	results->matrix_factorization_time += wall_time() - start_time;
	//
	// Solve the system.
	//
	start_time = wall_time();

	solve_system_ildl(B, L, S, P, -g, p);

	results->linear_solver_time += wall_time() - start_time;
}

void Solver::BKP_sym_ildl(const Eigen::MatrixXd& Hinput,
                          const Eigen::VectorXd& g,
                          Eigen::VectorXd* p,
                          SolverResults* results) const
{
	BKP_sym_ildl_generic(Hinput, g, p, results);
}

void Solver::BKP_sym_ildl(const Eigen::SparseMatrix<double>& Hinput,
                          const Eigen::VectorXd& g,
                          Eigen::VectorXd* p,
                          SolverResults* results) const
{
	BKP_sym_ildl_generic(Hinput, g, p, results);
}

}  // namespace spii
#endif // #ifndef USE_SYM_ILDL

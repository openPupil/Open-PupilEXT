// Petter Strandmark 2013.
//
// Rough test of sym-ildl.
//
#include <iostream>

#include <catch.hpp>

#include <Eigen/Dense>
#include <Eigen/LU>
#include <Eigen/Sparse>

#ifndef USE_SYM_ILDL

TEST_CASE("no-ildl-available")
{
	SUCCEED();
}

#else

extern "C" {
	#include "matrix.h"
	#include "matrix2.h"
	#include "sparse2.h"

	// Evil library...
	#undef min
	#undef max
	#undef catch
	#undef input
}

#include <lilc_matrix.h>

#include <spii/spii.h>
#include <spii/sym-ildl-conversions.h>

template<typename EigenMat>
MAT* Eigen_to_Meschach(const EigenMat& eigen_matrix)
{
	int m = static_cast<int>(eigen_matrix.rows());
	int n = static_cast<int>(eigen_matrix.cols());

	auto A = m_get(m, n);
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			A->me[i][j] = eigen_matrix(i, j);
		}
	}

	return A;
}

TEST_CASE("ildl-sym")
{
	using namespace std;
	using namespace Eigen;
	using namespace spii;

	const int n = 4;

	MatrixXd Aorg(n, n);
	Aorg.row(0) << 1, 2, 3, 1;
	Aorg.row(1) << 2, 6, 1, 8;
	Aorg.row(2) << 3, 1, 7, 6;
	Aorg.row(3) << 1, 8, 6, 6;
	REQUIRE((Aorg - Aorg.transpose()).norm() == 0);

	// Create sym-ildl matrix.
	lilc_matrix<double> Alilc;
	eigen_to_lilc(Aorg, &Alilc);

	// Convert matrix to Meschach format.
	auto Amat = Eigen_to_Meschach(Aorg);
	spii_at_scope_exit(m_free(Amat));

	cerr << "Original A = " << endl;
	cerr << Aorg << endl;
	cerr << "sym-ildl A = " << endl;
	cerr << Alilc << endl;
	cerr << "Meschach A = " << endl;
	m_foutput(stderr, Amat);

	cerr << endl << endl;

	// Factorize the matrix.
	PERM* pivot  = px_get(4);
	spii_at_scope_exit(px_free(pivot));
	PERM* block = px_get(4);
	spii_at_scope_exit(px_free(block));
	BKPfactor(Amat, pivot, block);

	// Print the results.
	cerr << "Meschach factorization = " << endl;
	m_foutput(stderr, Amat);
	px_foutput(stderr, block);
	px_foutput(stderr, pivot);
	cerr << endl << endl;

	cerr << "A is " << Alilc.n_rows() << " by " << Alilc.n_cols() << " with " << Alilc.nnz() << " non-zeros." << endl;
	lilc_matrix<double> Llilc;	     //<The lower triangular factor of A.
	vector<int> perm;	         //<A permutation vector containing all permutations on A.
	perm.reserve(Alilc.n_cols());
	block_diag_matrix<double> Dblockdiag; //<The diagonal factor of A.
	Alilc.sym_equil();
	Alilc.sym_rcm(perm);
	Alilc.sym_perm(perm);
	const double fill_factor = 1.0;
	const double tol         = 0.001; 

	Alilc.ildl(Llilc, Dblockdiag, perm, fill_factor, tol, 1.0);

	cerr << "L is " << Llilc.n_rows() << " by " << Llilc.n_cols() << " with " << Llilc.nnz() << " non-zeros." << endl;
	cerr << Llilc << endl;
	cerr << "D is " << Dblockdiag.n_rows() << " by " << Dblockdiag.n_cols() << " with " << Dblockdiag.nnz() << " non-zeros." << endl;
	cerr << Dblockdiag << endl;

	cerr << "P = ";
	for (auto val: perm) {
		cerr << val << " ";
	}
	cerr << endl;

	MyPermutation P(perm);
	MatrixXd I(perm.size(), perm.size());
	I.setIdentity();
	cerr << "P = " << endl << (P * I) << endl;

	auto L = lilc_to_eigen(Llilc);
	cerr << "L = " << endl << L << endl;

	auto D = block_diag_to_eigen(Dblockdiag);
	cerr << "D = " << endl << D << endl;

	auto S = diag_to_eigen(Alilc.S);
	cerr << "S = " << endl << S.toDenseMatrix() << endl;

	auto Btmp = lilc_to_eigen(Alilc, true);
	cerr << "B = " << endl << Btmp << endl;
	cerr << "P^T * S * A * S * P = " << endl << (P.transpose() * (S * Aorg * S) * P) << endl;
	cerr << "L * D * L^T = " << endl << (L * D * L.transpose()) << endl;

	cerr << endl << endl;
	auto SiPLDLtPtSi = S.inverse() * (P * L * D * L.transpose() * P.transpose()) * S.inverse();
	cerr << " S^-1 * P * L * D * L^T * P^T * S^-1 = " << endl 
	     << SiPLDLtPtSi << endl;

	REQUIRE((SiPLDLtPtSi - Aorg).norm() <= 1e-6);

	cerr << endl << endl;

	MatrixXd B(n, n);
	MatrixXd Q(n, n);
	VectorXd tau(n);
	VectorXd lambda(n);
	B.setZero();
	Q.setZero();

	SelfAdjointEigenSolver<MatrixXd> eigensolver;

	double delta = 1e-10;

	// Extract the block diagonal matrix.
	bool onebyone;
	for (int i = 0; i < n; i = (onebyone ? i+1 : i+2) ) {
		onebyone = (i == n-1 || D(i+1, i) == 0.0);

		if ( onebyone ) {
		    B(i, i) = D(i, i);
			lambda(i) = B(i, i);
			if (lambda(i) >= delta) {
				tau(i) = 0;
			}
			else {
				tau(i) = delta - lambda(i);
			}
			Q(i, i) = 1;
		}
		else {
		    auto a11 = D(i, i);
		    auto a22 = D(i+1, i+1);
		    auto a12 = D(i+1, i);
			B(i,   i)   = a11;
			B(i+1, i)   = a12;
			B(i,   i+1) = a12;
			B(i+1, i+1) = a22;
			eigensolver.compute(B.block(i, i, 2, 2));

			lambda(i)   = eigensolver.eigenvalues()(0);
			lambda(i+1) = eigensolver.eigenvalues()(1);
			for (int k = i; k <= i + 1; ++k) {
				if (lambda(k) >= delta) {
					tau(k) = 0;
				}
				else {
					tau(k) = delta - lambda(k);
				}
			}

			Q.block(i, i, 2, 2) = eigensolver.eigenvectors();
		}
	}

	cerr << "B = \n" << B << endl << endl;
	cerr << "lambda = " << tau.transpose() << endl << endl;
	cerr << "tau = " << tau.transpose() << endl << endl;
	cerr << "Q = \n" << Q << endl << endl;
	cerr << "Q*lambda*Q^T = \n" << Q * lambda.asDiagonal() * Q.transpose() << endl << endl;

	// Check that the block-wise eigendecomposition was correct.
	CHECK(((B - Q * lambda.asDiagonal() * Q.transpose()).norm()) < 1e-10);

	MatrixXd F = Q * tau.asDiagonal() * Q.transpose();
	cerr << "F = Q*tau*Q^T = \n" << F << endl << endl;
	cerr << "B + F = \n" << B + F << endl << endl;


	// Check that solver works correctly.
	VectorXd b(4);
	b(0) = 2.0;
	b(1) = 1.0;
	b(2) = 3.0;
	b(3) = -1.0;

	VectorXd xorg = Aorg.lu().solve(b);
	VectorXd x;
	solve_system_ildl(Dblockdiag, Llilc, S, P, b, &x);
	CAPTURE(xorg.transpose());
	CAPTURE(x.transpose());
	CHECK( (xorg - x).norm() <= 1e-6 );
}

TEST_CASE("ildl-sym-sparse")
{
	using namespace std;
	using namespace Eigen;
	using namespace spii;

	int n = 8;
	vector<Triplet<double>> triplets;
	auto add_element = [&](int i, int j, double v)
	{
		triplets.emplace_back(i - 1, j- 1, v);
	};
	// Matlab indices which start at 1.
	add_element(1,1, 19);
	add_element(2,1,  7);
	add_element(4,1, 20);
	add_element(6,1,-15);
	add_element(8,1, -6);
	add_element(1,2,  7);
	add_element(2,2, -8);
	add_element(4,2,  4);
	add_element(6,2, -3);
	add_element(8,2, -3);
	add_element(4,3,  4);
	add_element(8,3,  1);
	add_element(1,4, 20);
	add_element(2,4,  4);
	add_element(3,4,  4);
	add_element(4,4, 23);
	add_element(6,4,-12);
	add_element(8,4,  4);
	add_element(5,5,-10);
	add_element(1,6,-15);
	add_element(2,6, -3);
	add_element(4,6,-12);
	add_element(6,6, -1);
	add_element(7,7, -1);
	add_element(1,8, -6);
	add_element(2,8, -3);
	add_element(3,8,  1);
	add_element(4,8,  4);

	SparseMatrix<double> Aorg(n, n);
	Aorg.setFromTriplets(begin(triplets), end(triplets));

	cerr << Aorg << endl;

	// Create sym-ildl matrix.
	lilc_matrix<double> Alilc;
	eigen_to_lilc(Aorg, &Alilc);

	SparseMatrix<double> A;
	lilc_to_eigen(Alilc, &A, true);

	cerr << "Original A = " << endl;
	cerr << Aorg << endl;
	cerr << "sym-ildl A = " << endl;
	cerr << Alilc << endl;
	cerr << "A converted back = " << endl;
	cerr << A << endl;

	SparseMatrix<double> Zero = A - Aorg;
	CHECK(Zero.sum() == 0);

	cerr << "A is " << Alilc.n_rows() << " by " << Alilc.n_cols() << " with " << Alilc.nnz() << " non-zeros." << endl;
	lilc_matrix<double> Llilc;	     //<The lower triangular factor of A.
	vector<int> perm;	         //<A permutation vector containing all permutations on A.
	perm.reserve(Alilc.n_cols());
	block_diag_matrix<double> Dblockdiag; //<The diagonal factor of A.
	Alilc.sym_equil();
	Alilc.sym_rcm(perm);
	Alilc.sym_perm(perm);
	const double fill_factor = 1.0;
	const double tol         = 0.001;

	Alilc.ildl(Llilc, Dblockdiag, perm, fill_factor, tol, 1.0);

	cerr << "L is " << Llilc.n_rows() << " by " << Llilc.n_cols() << " with " << Llilc.nnz() << " non-zeros." << endl;
	cerr << Llilc << endl;
	cerr << "D is " << Dblockdiag.n_rows() << " by " << Dblockdiag.n_cols() << " with " << Dblockdiag.nnz() << " non-zeros." << endl;
	cerr << Dblockdiag << endl;

	cerr << "P = ";
	for (auto val: perm) {
		cerr << val << " ";
	}
	cerr << endl;

	MyPermutation P(perm);
	MatrixXd I(perm.size(), perm.size());
	I.setIdentity();
	cerr << "P = " << endl << (P * I) << endl;

	auto L = lilc_to_eigen(Llilc);
	cerr << "L = " << endl << L << endl;

	SparseMatrix<double> D;
	block_diag_to_eigen(Dblockdiag, &D);
	cerr << "D = " << endl << D << endl;

	auto S = diag_to_eigen(Alilc.S);
	cerr << "S = " << endl << S.toDenseMatrix() << endl;

	SparseMatrix<double> Btmp;
	lilc_to_eigen(Alilc, &Btmp, true);
	cerr << "B = " << endl << Btmp.toDense() << endl;
	cerr << "P^T * S * A * S * P = " << endl << (P.transpose() * (S * A.toDense() * S) * P) << endl;
	cerr << "L * D * L^T = " << endl << (L * D.toDense() * L.transpose()) << endl;

	cerr << endl << endl;
	auto SiPLDLtPtSi = S.inverse() * (P * L * D * L.transpose() * P.transpose()) * S.inverse();
	cerr << " S^-1 * P * L * D * L^T * P^T * S^-1 = " << endl 
	     << SiPLDLtPtSi << endl;

	REQUIRE((SiPLDLtPtSi - Aorg.toDense()).norm() <= 1e-6);

	cerr << endl << endl;
}

#endif // #ifndef USE_SYM_ILDL

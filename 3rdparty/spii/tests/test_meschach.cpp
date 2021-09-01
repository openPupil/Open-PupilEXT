
#include <functional>
#include <future>
#include <iostream>
#include <random>
#include <thread>

#include <catch.hpp>

#include <Eigen/Dense>

#include <spii/spii.h>

extern "C" {
	#include "matrix.h"
	#include "matrix2.h"
	#include "sparse2.h"
	#undef min
	#undef max
	#undef catch
}

template<typename EigenMat>
MAT* Eigen_to_Meschach(const EigenMat& eigen_matrix)
{
	auto m = eigen_matrix.rows();
	auto n = eigen_matrix.cols();

	auto A = m_get(int(m), int(n));
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			A->me[i][j] = eigen_matrix(i, j);
		}
	}

	return A;
}

TEST_CASE("BKP-dense", "")
{
	using namespace std;
	using namespace Eigen;

	int n = 4;

	VectorXd x(4), b(4);
	b << 1, 2, 3, 4;

	MatrixXd A(4, 4);
	A.row(0) << 1, 2, 3, 1;
	A.row(1) << 2, 6, 1, 8;
	A.row(2) << 3, 1, 7, 6;
	A.row(3) << 1, 8, 6, 6;
	REQUIRE((A - A.transpose()).norm() == 0);

	// Sanity check.
	x = A.lu().solve(b);
	INFO(x);
	CHECK((A * x - b).norm() <= 1e-10);

	// Convert matrix to Meschach format.
	auto Amat = Eigen_to_Meschach(A);
	m_foutput(stderr, Amat);

	// Factorize the matrix.
	PERM* pivot  = px_get(4);
	PERM* block = px_get(4);
	spii_at_scope_exit(
		px_free(pivot);
		px_free(block);
	);
	BKPfactor(Amat, pivot, block);

	// Print the results.
	m_foutput(stderr, Amat);
	spii_at_scope_exit( m_free(Amat); );
	px_foutput(stderr, block);
	px_foutput(stderr, pivot);
	cerr << endl << endl;

	// Solve the linear system.
	VEC* bvec = v_get(4);
	spii_at_scope_exit( v_free(bvec); );
	for (int i = 0; i < 4; ++i) {
		bvec->ve[i] = b(i);
	}
	VEC* xvec = BKPsolve(Amat, pivot, block, bvec, nullptr);
	spii_at_scope_exit( v_free(xvec); );
	for (int i = 0; i < 4; ++i) {
		x(i) = xvec->ve[i];
	}
	INFO(x);
	INFO(A * x);
	CHECK((A * x - b).norm() <= 1e-10);

	MatrixXd B(4, 4);
	MatrixXd Q(4, 4);
	VectorXd tau(4);
	VectorXd lambda(4);
	B.setZero();
	Q.setZero();

	SelfAdjointEigenSolver<MatrixXd> eigensolver;

	double delta = 1e-10;

	// Extract the block diagonal matrix.
	int onebyone;
	for (int i = 0; i < n; i = onebyone ? i+1 : i+2 ) {
		onebyone = ( block->pe[i] == i );
		if ( onebyone ) {
		    B(i, i) = m_entry(Amat,i,i);
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
		    auto a11 = m_entry(Amat,i,i);
		    auto a22 = m_entry(Amat,i+1,i+1);
		    auto a12 = m_entry(Amat,i+1,i);
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
}

#ifdef USE_OPENMP
TEST_CASE("BKP-dense-threadsafe")
{
	using namespace std;
	using namespace Eigen;

	auto stress_test = [](unsigned seed)
	{
		mt19937_64 engine(seed);
		auto rand = bind(uniform_int_distribution<int>(-10, 10), ref(engine));
		const int n = 400;

		MatrixXd A(n, n);
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				A(i, j) = rand();
			}
		}
		A = A.transpose() * A;
		auto Amat = Eigen_to_Meschach(A);
		spii_at_scope_exit( m_free(Amat); );

		// Factorize the matrix.
		PERM* pivot = px_get(n);
		spii_at_scope_exit( px_free(pivot); );
		PERM* block = px_get(n);
		spii_at_scope_exit( px_free(block); );
		BKPfactor(Amat, pivot, block);

		VectorXd b(n);
		VEC* bvec = v_get(n);
		spii_at_scope_exit( v_free(bvec); );
		for (int i = 0; i < n; ++i) {
			b(i) = rand();
			bvec->ve[i] = b(i);
		}

		for (int iteration = 1; iteration <= 20; ++iteration) {
			VEC* xvec = BKPsolve(Amat, pivot, block, bvec, 0);
			VectorXd x(n);
			for (int i = 0; i < n; ++i) {
				x(i) = xvec->ve[i];
			}
			v_free(xvec);
			double err = (A * x - b).norm() / b.norm();
			if (err > 1e-6) {
				throw std::runtime_error("Not thread-safe!");
			}
		}		
	};


	try {
		auto f1 = async(launch::async, stress_test, 1);
		auto f2 = async(launch::async, stress_test, 2);
		auto f3 = async(launch::async, stress_test, 3);
		auto f4 = async(launch::async, stress_test, 4);
		auto f5 = async(launch::async, stress_test, 5);
		f1.get();
		f2.get();
		f3.get();
		f4.get();
		f5.get();
		SUCCEED();
	}
	catch (...) {
		FAIL();
	}
}
#endif

/*
TEST_CASE("BKP-sparse", "")
{
	int m = 5;
	int n = 5;
	int deg = 5;
	SPMAT* A = sp_get(m, n, deg);

	int I[25]    = {1,2,3,4,5,1,2,3,4,5,1,2,3,4,5,1,2,3,4,5,1,2,3,4,5};
	int J[25]    = {1,1,1,1,1,2,2,2,2,2,3,3,3,3,3,4,4,4,4,4,5,5,5,5,5};
	double V[25] = {1,7,3,5,1,5,2,6,6,7,4,1,2,8,1,7,5,9,1,4,1,9,0,7,7};

	for (int i = 0; i < 25; ++i) {
		if (V[i] != 0) {
			sp_set_val(A, I[i] - 1, J[i] - 1, V[i]);
		}
	}

	sp_foutput2(stderr, A);

	SPMAT* B = sp_copy(A);
	PERM* pivot  = px_get(5);
	PERM* blocks = px_get(5);
	spBKPfactor(B, pivot, blocks, 1e-16);

	sp_fo1utput2(stderr, B);
	px_foutput(stderr, pivot);
	px_foutput(stderr, blocks);

	SP_FREE(A);
	SP_FREE(B);
}
*/
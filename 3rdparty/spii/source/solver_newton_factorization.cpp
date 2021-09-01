// Petter Strandmark 2012.

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

extern "C" {
	#include "matrix.h"
	#include "matrix2.h"
	#undef min
	#undef max
	#undef catch
	#undef SPARSE
}

#include <spii/spii.h>
#include <spii/solver.h>

namespace spii {

template<typename EigenMat>
void Eigen_to_Meschach(const EigenMat& eigen_matrix, MAT* A)
{
	auto m = eigen_matrix.rows();
	auto n = eigen_matrix.cols();

	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			A->me[i][j] = eigen_matrix(i, j);
		}
	}
}

struct FactorizationCacheInternal
{
	PERM* pivot;
	PERM* block;
	MAT* Hmat;
	VEC* x;
	VEC* b;
};

FactorizationCache::FactorizationCache(int n)
{
	auto cache = new FactorizationCacheInternal;
	cache->pivot = px_get(n);
	cache->block = px_get(n);
	cache->Hmat  = m_get(n, n);
	cache->x     = v_get(n);
	cache->b     = v_get(n);
	this->data = cache;
}

FactorizationCache::~FactorizationCache()
{
	v_free(this->data->b);
	v_free(this->data->x);
	m_free(this->data->Hmat);
	px_free(this->data->pivot);
	px_free(this->data->block);
	delete this->data;
}

void Solver::BKP_dense(const Eigen::MatrixXd& H,
                       const Eigen::VectorXd& g,
                       const FactorizationCache& cache_input,
                       Eigen::VectorXd* p,
                       SolverResults* results) const
{
	using namespace Eigen;
	double start_time = wall_time();

	auto cache = cache_input.data;
	auto n = H.rows();

	Eigen_to_Meschach(H, cache->Hmat);

	//m_foutput(stderr, cache->Hmat);

	BKPfactor(cache->Hmat, cache->pivot, cache->block);

	//m_foutput(stderr, cache->Hmat);

	MatrixXd B(n, n);
	MatrixXd Q(n, n);
	VectorXd tau(n);
	VectorXd lambda(n);
	B.setZero();
	Q.setZero();

	SelfAdjointEigenSolver<MatrixXd> eigensolver;

	double delta = 1e-12;

	int onebyone;
	for (int i = 0; i < n; i = onebyone ? i+1 : i+2 ) {
		onebyone = ( cache->block->pe[i] == i );
		if ( onebyone ) {
		    B(i, i) = m_entry(cache->Hmat, i, i);
			lambda(i) = B(i, i);
			if (lambda(i) >= delta) {
				tau(i) = 0;
			}
			else {
				tau(i) = delta - (1.0 + delta) * lambda(i);
			}
			Q(i, i) = 1;
		}
		else {
		    auto a11 = m_entry(cache->Hmat, i, i);
		    auto a22 = m_entry(cache->Hmat, i+1, i+1);
		    auto a12 = m_entry(cache->Hmat, i+1, i);
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
					tau(k) = delta - (1.0 + delta) * lambda(k);
				}
			}

			Q.block(i, i, 2, 2) = eigensolver.eigenvectors();
		}
	}

	//std::cerr << "B = \n" << B << "\n\n";
	//std::cerr << "F = \n" <<  Q * tau.asDiagonal() * Q.transpose() << "\n\n";

	B = B + Q * tau.asDiagonal() * Q.transpose();

	//std::cerr << "B = \n" << B << "\n\n";

	for (int i = 0; i < n; i = onebyone ? i+1 : i+2 ) {
		onebyone = ( cache->block->pe[i] == i );
		if ( onebyone ) {
		   m_entry(cache->Hmat, i, i) = B(i, i);
		}
		else {
		    m_entry(cache->Hmat, i,   i)   = B(i,   i);
		    m_entry(cache->Hmat, i+1, i+1) = B(i+1, i+1);
		    m_entry(cache->Hmat, i+1, i)   = B(i+1, i);
			m_entry(cache->Hmat, i,   i+1) = B(i,   i+1);
		}
	}

	results->matrix_factorization_time += wall_time() - start_time;
	start_time = wall_time();

	//m_foutput(stderr, cache->Hmat);

	for (int i = 0; i < n; ++i) {
		cache->b->ve[i] = -g(i);
	}
	BKPsolve(cache->Hmat, cache->pivot, cache->block, cache->b, cache->x);
	for (int i = 0; i < n; ++i) {
		(*p)(i) = cache->x->ve[i];
	}

	results->linear_solver_time += wall_time() - start_time;
	start_time = wall_time();

	results->matrix_factorization_time += wall_time() - start_time;
}

}  // namespace spii

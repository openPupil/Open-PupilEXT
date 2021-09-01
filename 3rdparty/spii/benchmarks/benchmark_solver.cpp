
#include <functional>
#include <iostream>
#include <random>

#include <spii/auto_diff_term.h>
#include <spii/function.h>
#include <spii/solver.h>
using namespace spii;

#include "hastighet.h"

// One term in the negative log-likelihood function for
// a one-dimensional Gaussian distribution.
struct NegLogLikelihood
{
	double sample;
	NegLogLikelihood(double sample)
	{
		this->sample = sample;
	}

	template<typename R>
	R operator()(const R* const mu, const R* const sigma) const
	{
		R diff = (*mu - sample) / *sigma;
		return 0.5 * diff*diff + log(*sigma);
	}
};

template<typename SolverClass>
class LikelihoodBenchmark :
	public hastighet::Test
{
public:
	Function f;
	double mu;
	double sigma;
	Eigen::VectorXd x, g;
	Eigen::MatrixXd H;
	double out;
	SolverClass solver;
	SolverResults results;

	LikelihoodBenchmark() : 
		x(2),
		g(2)
	{
		std::mt19937 prng(unsigned(1));
		std::normal_distribution<double> normal;
		auto randn = std::bind(normal, prng);

		mu    = 5.0;
		sigma = 3.0;

		f.add_variable(&mu, 1);
		f.add_variable(&sigma, 1);

		for (int i = 0; i < 10000; ++i) {
			double sample = sigma*randn() + mu;
			f.add_term(std::make_shared<AutoDiffTerm<NegLogLikelihood, 1, 1>>(sample), &mu, &sigma);
		}

		f.copy_user_to_global(&x);
		f.set_number_of_threads(1);

		solver.log_function = [](const std::string&) { };
		solver.maximum_iterations = 1;
	}
};

typedef LikelihoodBenchmark<NewtonSolver> LikelihoodBenchmarkNewtonSolver;
BENCHMARK_F(LikelihoodBenchmarkNewtonSolver, solver_1_newton_iter)
{
	mu = 5.0;
	sigma = 1.0;
	solver.solve(f, &results);
}

typedef LikelihoodBenchmark<LBFGSSolver> LikelihoodBenchmarkLBFGSSolver;
BENCHMARK_F(LikelihoodBenchmarkLBFGSSolver, solver_1_lbfgs_iter)
{
	mu = 5.0;
	sigma = 1.0;
	solver.solve(f, &results);
}

struct LennardJonesTerm
{
	template<typename R>
	R operator()(const R* const p1, const R* const p2) const
	{
		R dx = p1[0] - p2[0];
		R dy = p1[1] - p2[1];
		R dz = p1[2] - p2[2];
		R r2 = dx*dx + dy*dy + dz*dz;
		R r6  = r2*r2*r2;
		R r12 = r6*r6;
		return 1.0 / r12 - 2.0 / r6;
	}
};

template<typename SolverClass>
class LennardJonesBenchmark :
	public hastighet::Test
{
public:
	Function potential;
	std::vector<Eigen::Vector3d> points;
	std::vector<Eigen::Vector3d> org_points;
	Eigen::VectorXd x, g;
	Eigen::MatrixXd H;
	SolverClass solver;
	SolverResults results;

	LennardJonesBenchmark() : 
		points(100)
	{
		std::mt19937 prng(1);
		std::normal_distribution<double> normal;
		auto randn = std::bind(normal, prng);

		auto N = points.size();
		auto n = int(std::ceil(std::pow(double(N), 1.0/3.0)));

		// Initial position is a cubic grid with random pertubations.
		for (int i = 0; i < N; ++i) {
			int x =  i % n;
			int y = (i / n) % n;
			int z = (i / n) / n;

			potential.add_variable(&points[i][0], 3);
			points[i][0] = x + 0.05 * randn();
			points[i][1] = y + 0.05 * randn();
			points[i][2] = z + 0.05 * randn();
		}

		for (int i = 0; i < N; ++i) {
			for (int j = i + 1; j < N; ++j) {
				potential.add_term(
					std::make_shared<AutoDiffTerm<LennardJonesTerm, 3, 3>>(),
					&points[i][0],
					&points[j][0]);
			}
		}

		x.resize(potential.get_number_of_scalars());
		potential.copy_user_to_global(&x);
		g.resize(potential.get_number_of_scalars());
		org_points = points;

		potential.set_number_of_threads(1);

		solver.log_function = [](const std::string&) { };
		solver.maximum_iterations = 1;
	}
};

typedef LennardJonesBenchmark<NewtonSolver> LennardJonesBenchmarkNewtonSolver;
BENCHMARK_F(LennardJonesBenchmarkNewtonSolver, solver_1_newton_iter)
{
	points = org_points;
	solver.solve(potential, &results);
}

typedef LennardJonesBenchmark<LBFGSSolver> LennardJonesBenchmarkLBFGSSolver;
BENCHMARK_F(LennardJonesBenchmarkLBFGSSolver, solver_1_lbfgs_iter)
{
	points = org_points;
	solver.solve(potential, &results);
}

int main(int argc, char** argv)
{
	hastighet::Benchmarker::RunAllTests(argc, argv);
}

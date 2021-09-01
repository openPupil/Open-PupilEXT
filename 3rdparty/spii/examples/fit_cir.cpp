// Petter Strandmark 2012.
//
// This example performs inference in the Cox–Ingersoll–Ross model
// (CIR model) which is a stochastic differential equation of the
// form
//
//    dr(t) = kappa*(theta - r(t))*dt + sigma*sqrt(r(t))*dW(r).
//
// The goal is to estimate kappa, theta and sigma from observed data.
// The CIR model is a model for interest rates.
//
// This will be done in two steps. First, an approximate log-likelihood
// function will be computed with a discretization of the PDE equal to
// the observed time interval. Second, using the first estimate as an
// initial estimate, a Monte-Carlo simulation will be used to compute
// a better likelihood function.
//
// This example illustrates that automatic differentiation can be used
// to compute first and second order derivatives of very complicated
// objective functions.
//
// The parameters optimized over are (log(kappa), log(theta), log(sigma)).
// This simple reparametrization prevents them from becoming negative.

#include <cmath>
#include <functional>
#include <iostream>
#include <random>

#include <spii/auto_diff_term.h>
#include <spii/solver.h>

using namespace spii;

// Probability density for a 1D normal distribution.
template<typename R>
R normpdf(const R& x, const R& mu, const R& sigma)
{
	R diff = (mu - x) / (sqrt(2.0)*sigma);
	return 1.0 / (sigma * sqrt(2.0 * 3.141592))
	       * exp(- diff * diff);
}

// Log probability density for a 1D normal distribution.
template<typename R>
R lognormpdf(const R& x, const R& mu, const R& sigma)
{
	R diff = (mu - x) / (sqrt(2.0)*sigma);
	return - log(sigma * sqrt(2.0 * 3.141592))
	       - diff * diff;
}

struct CIRApproximateLL
{
	const std::vector<double>& data;
	CIRApproximateLL(const std::vector<double>& data_in):
		data(data_in)
	{
	}

	template<typename R>
	R operator()(R* param) const
	{
		// Retrieve the three parameters.
		R kappa = exp(param[0]);
		R theta = exp(param[1]);
		R sigma  = exp(param[2]);

		R L = 0.0;
		for (size_t i = 0; i < data.size() - 1; ++i) {
			R eps_iplus1 = data[i+1] - data[i] - kappa*theta + kappa*data[i];
			L += lognormpdf<R>(eps_iplus1, 0, sigma * sqrt(data[i]));
		}

		// Return the negative log-likelihood since we are minimizing instead
		// of maximizing.
		return -L;
	}
};

typedef std::mt19937_64 Rng;

struct SimulateCIR
{
	double r0;
	double r1;
	int M;
	int K;

	static Rng::result_type seed;
	Rng::result_type local_seed;

	SimulateCIR(double r0, double r1, int M, int K)
	{
		this->r0 = r0;
		this->r1 = r1;
		this->M = M;
		this->K = K;
		this->local_seed = seed++;
	}

	template<typename R>
	R operator()(R* param) const
	{
		// The random number generator
		Rng prng(this->local_seed);
		std::normal_distribution<double> normal;
		auto randn = std::bind(normal, prng);

		// Retrieve the three parameters.
		R kappa = exp(param[0]);
		R theta = exp(param[1]);
		R sigma = exp(param[2]);

		double d = 1.0 / double(this->M);

		// Simulate K trajectories from r0 to r1
		R p = 0.0;
		for (int k = 1; k <= this->K; ++k) {
			R r = this->r0;
			// Simulate the trajectory in M-1 steps.
			for (int i = 1; i <= this->M - 1; ++i) {
				r = r + kappa*(theta - r)*d + sigma * sqrt(r*d) * randn();
				// This is to make sure r never becomes negative (which is
				// impossible in the continuous case).
				if (to_double(r) < 1e-8) {
					r = 1e-8;
				}
			}
			// Add the probability to go from r to r1.
			p += normpdf<R>(this->r1, r + kappa*(theta - r)*d, sqrt(d*r)*sigma);
		}
		// The estimated probability is the average of all trajectories.
		p /= double(K);

		// Return the negative log-likelihood since we are minimizing instead
		// of maximizing.
		return -log(p);
	}
};

Rng::result_type SimulateCIR::seed = Rng::default_seed;

int main_function()
{
	// Read the observed data from standard input.
	std::vector<double> data;
	std::cerr << "Reading data from stdin... (use CIR.txt for example data)\n";
	while (true) {
		double r;
		std::cin >> r;
		if (! std::cin) {
			break;
		}
		data.push_back(r);
	}
	std::cerr << "Read " << data.size() << " data points\n";

	// These are the initial estimates of the parameters.
	// Using these with the simulated log-likelihood does
	// not work very well.
	double kappa = 1.0;
	double theta = 1.0;
	double sigma = 1.0;
	std::vector<double> param(3);
	param[0] = std::log(kappa);
	param[1] = std::log(theta);
	param[2] = std::log(sigma);

	// Use the standard solver settings.
	NewtonSolver solver;
	SolverResults results;

	// First, optimize the approximate log-likelihood to get a
	// good initial estimate for the simulated log-likelihood.
	Function approx_f;
	approx_f.add_variable(&param[0], 3);
	approx_f.add_term(std::make_shared<AutoDiffTerm<CIRApproximateLL, 3>>(data), &param[0]);
	solver.solve(approx_f, &results);

	// Print the estimated paramters.
	kappa = std::exp(param[0]);
	theta = std::exp(param[1]);
	sigma = std::exp(param[2]);
	std::cout << "Estimated:" << std::endl;
	std::cout << "kappa = " << kappa << std::endl;
	std::cout << "theta = " << theta << std::endl;
	std::cout << "sigma = " << sigma << std::endl;

	// M is the number of time-steps between each observed sample
	// used when simulating trajectories.
	int M = 10;
	// K is the number of trajectories simulated between each
	// observed data point in order to estimate the transitional
	// probability.
	int K = 100;

	// Create the objective function with one term for each pair of
	// adjacent observed samples.
	Function f;
	f.add_variable(&param[0], 3);
	for (size_t i = 0; i < data.size() - 1; ++i) {
		f.add_term(std::make_shared<AutoDiffTerm<SimulateCIR, 3>>(data[i], data[i + 1], M, K), &param[0]);
	}

	solver.function_improvement_tolerance = 1e-6;
	solver.solve(f, &results);

	std::cerr << results;
	f.print_timing_information(std::cerr);

	kappa = std::exp(param[0]);
	theta = std::exp(param[1]);
	sigma = std::exp(param[2]);
	std::cout << "Estimated:" << std::endl;
	std::cout << "kappa = " << kappa << std::endl;
	std::cout << "theta = " << theta << std::endl;
	std::cout << "sigma = " << sigma << std::endl;

	return 0;
}

int main()
{
	try {
		return main_function();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}

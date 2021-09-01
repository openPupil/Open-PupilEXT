// Petter Strandmark 2014.
//
// This example illustrates how function objects can be
// reused with different data.
//
// Computes a confidence interval for the mean of a Gaussian
// distribution. (This ML-problem can be solved in closed-
// form, so no optimization is really neccessary.)
//

#include <functional>
#include <iostream>
#include <random>

#include <spii/auto_diff_term.h>
#include <spii/interval_term.h>
#include <spii/solver.h>
#include <spii/transformations.h>
#include <spii/color.h>

//
// This class does the same thing as estimate_mu, but
// uses the same Function object and therefore saves
// the overhead of creating it over and over again.
//
class MuEstimator
{
public:
	// One term in the negative log-likelihood function for
	// a one-dimensional Gaussian distribution. Using a
	// pointer to the data so the underlying values may
	// change.
	struct NegLogLikelihoodUsingPointer
	{
		double* sample;
		NegLogLikelihoodUsingPointer(double* sample)
		{
			this->sample = sample;
		}

		template<typename R>
		R operator()(const R* const mu, const R* const sigma) const
		{
			R diff = (*mu - *sample) / *sigma;
			return 0.5 * diff*diff + log(*sigma);
		}
	};

	MuEstimator(size_t data_size)
		: samples(data_size, 0.0)
	{
		solver.log_function = nullptr;

		f.add_variable(&mu, 1);
		f.add_variable_with_change<spii::GreaterThanZero>(&sigma, 1, 1);

		for (auto& sample: samples) {
			f.add_term(std::make_shared<spii::IntervalTerm<NegLogLikelihoodUsingPointer, 1, 1>>(&sample), &mu, &sigma);
		}
	}

	double operator()(const std::vector<double>& new_samples)
	{
		spii_assert(samples.size() == new_samples.size());
		double ground_truth_ML_estimator = 0;
		for (size_t i = 0; i < samples.size(); ++i) {
			samples[i] = new_samples[i];
			ground_truth_ML_estimator += samples[i];
		}
		ground_truth_ML_estimator /= samples.size();

		// Instead starting at the previous point may introduce
		// a bias. (but increases performance)
		mu    = 0;
		sigma = 1;
		solver.solve(f, &results);

		accumulated_error += std::abs(mu - ground_truth_ML_estimator);
		num_optimizations++;
		return mu;
	}

	double get_average_error() const
	{
		return accumulated_error / num_optimizations;
	}

private:
	std::vector<double> samples;
	double mu    = 0;
	double sigma = 1;
	spii::Function f;
	spii::LBFGSSolver solver;
	spii::SolverResults results;

	double accumulated_error = 0;
	int num_optimizations = 0;
};

template<typename T, typename ComputeSample, typename random_engine>
void generate_samples(const std::vector<T>& data,
                      ComputeSample& compute_sample,
                      std::vector<double>* samples,
					  random_engine* engine,
                      std::size_t iterations = 10000)
{
	using namespace std;

	auto n = data.size();
	vector<T> data_copy(n);
	samples->resize(iterations);

	std::uniform_int_distribution<size_t> rand_index(0, n - 1);

	for (size_t iter = 0; iter < iterations; ++iter) {
		for (size_t i = 0; i < n; ++i) {
			data_copy[i] = data[rand_index(*engine)];
		}
		(*samples)[iter] = compute_sample(data_copy);
	}

	sort(begin(*samples), end(*samples));
}

template<typename T>
std::pair<double, double> compute_interval_simple(const std::vector<T>& samples,
                                                  double p = 0.95,
                                                  bool lower_limit = false,
                                                  bool upper_limit = true)
{
	using namespace std;

	if (!lower_limit && !upper_limit) {
		throw runtime_error("No limits requested.");
	}
	else if (lower_limit && upper_limit) {
		// Split the probability between upper and lower half.
		p = 1.0 - (1.0 - p) / 2.0;
	}

	auto interval = make_pair( - numeric_limits<double>::infinity(),
                                 numeric_limits<double>::infinity());

	if (lower_limit) {
		interval.first = samples.at( size_t((samples.size() + 1) * (1 - p) - 1) );
	}
	if (upper_limit) {
		interval.second  = samples.at( size_t((samples.size() + 1) * p - 1) );
	}
	return interval;
}

int main_function()
{
	using namespace std;

	mt19937_64 prng(1u);
	double mu    = 5.0;
	double sigma = 3.0;
	normal_distribution<double> normal(mu, sigma);

	const int n_data    = 100;
	const int n_samples = 1000;

	// Generate the data with the gives mu and sigma.
	vector<double> data_samples;
	for (int i = 0; i < n_data; ++i) {
		data_samples.emplace_back(normal(prng));
	}

	spii::Timer timer("Generating samples");
		vector<double> mu_samples;
		MuEstimator mu_estimator(n_data);
		generate_samples(data_samples, mu_estimator, &mu_samples, &prng, n_samples);
	timer.OK();
	auto interval = compute_interval_simple(mu_samples, 0.95, true, true);
	cout << spii::to_string(interval) << endl;
	cout << "Average error (compared to closed-form ML-estimator): " << mu_estimator.get_average_error() << endl;
	cout << "Average time per optimization problem: " << 1e6 * timer.get_elapsed_time() / n_samples << " micros." << endl;

	return 0;
}

int main()
{
	try {
		return main_function();
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << '\n';
		return 1;
	}
}

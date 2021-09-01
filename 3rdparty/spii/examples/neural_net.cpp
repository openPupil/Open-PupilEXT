// Petter Strandmark 2013.
//
// Trains a small artificial neural network using L-BFGS.
// Learns the xor function from two inputs to one output
// variable.
//
// This example illustrates how functions of matrices can
// be optimized using automatic differentiations.

#include <iostream>
#include <random>
#include <stdexcept>

#include <spii/auto_diff_term.h>
#include <spii/constrained_function.h>
#include <spii/solver.h>

template<typename R, int rows>
void sigmoid_inplace(Eigen::Matrix<R, rows, 1>& v)
{
	using std::exp;
	for (int i = 0; i < rows; ++i) {
		v[i] = 1.0 / (1.0 + exp(-v[i]));
	}
}

template<typename R, int n_input, int n_hidden, int n_output>
Eigen::Matrix<R, n_output, 1> neural_network_classify(const Eigen::Matrix<double, n_input, 1>& input,
                                                      const R* const W1_data,
                                                      const R* const W2_data)
{
	Eigen::Map<const Eigen::Matrix<R, n_hidden, n_input>>  W1(W1_data);
	Eigen::Map<const Eigen::Matrix<R, n_output, n_hidden>> W2(W2_data);

	Eigen::Matrix<R, n_input, 1> input_R;
	// input.cast<R>() does not work on GCC 4.8.2...
	for (int i = 0; i < n_input; ++i) {
		input_R(i) = input(i);
	}

	Eigen::Matrix<R, n_hidden, 1> hidden = W1 * input_R;
	sigmoid_inplace(hidden);
	Eigen::Matrix<R, n_output, 1> output = W2 * hidden;
	sigmoid_inplace(output);
	return output;
}

template<int n_input, int n_hidden, int n_output>
class NeuralNetQuadraticLoss
{
public:
	NeuralNetQuadraticLoss(const Eigen::Matrix<double, n_input, 1>& input_,
	                       const Eigen::Matrix<double, n_output, 1>& desired_output_)
		: input{input_},
		  desired_output{desired_output_}
	{ }

	template<typename R>
	R operator()(const R* const W1_data, const R* const W2_data) const
	{
		// Classify the input using the neural network given
		// as input to this function.
		auto output = neural_network_classify<R, n_input, n_hidden, n_output>(input, W1_data, W2_data);

		// Compute the squared error.
		R squared_sum = 0.0;
		for (int i = 0; i < n_output; ++i) {
			auto delta = output[i] - R(desired_output[i]);
			squared_sum += delta*delta;
		}
		return squared_sum;
	}

private:
	const Eigen::Matrix<double, n_input, 1> input;
	const Eigen::Matrix<double, n_output, 1> desired_output;
};

// Number of nodes in the three-layer network.
const int n_input = 2;
const int n_hidden = 4;
const int n_output = 1;

int main_function()
{
	using namespace spii;
	using namespace std;

	vector<double> W1(n_hidden * n_input, 1.0);
	vector<double> W2(n_output * n_hidden, 0.0);

	std::mt19937_64 engine;
	std::normal_distribution<double> rand(0.0, 1.0);
	for (auto& w: W1) {
		w = rand(engine);
	}
	for (auto& w : W2) {
		w = rand(engine);
	}

	Function function;
	Eigen::Matrix<double, n_input, 1> input;
	Eigen::Matrix<double, n_output, 1> output;

	typedef NeuralNetQuadraticLoss<n_input, n_hidden, n_output> NeuralNet;
	typedef AutoDiffTerm<NeuralNet, n_hidden * n_input, n_output * n_hidden> NeuralNetTerm;

	vector<vector<double>> training = {
		{0, 0, 0},
		{0, 1, 1},  // {input1, input2, output1}
		{1, 0, 1},
		{1, 1, 0},
	};

	for (const auto& example: training) {
		for (double eps1 = -0.1; eps1 <= 0.1; eps1 += 0.02) {
			for (double eps2 = -0.1; eps2 <= 0.1; eps2 += 0.02) {
				// Do not traing on exactly on (0, 0), ... (1, 1)
				// Only on slight permutations.
				if (eps1 == 0 && eps2 == 0)
					continue;

				input[0] = example[0] + eps1;
				input[1] = example[1] + eps2;
				//input[2] = 1.0;
				output[0] = example[2];
				auto term = make_shared<NeuralNetTerm>(input, output);
				function.add_term(term, W1.data(), W2.data());
			}
		}
	}

	cout << "Using " << function.get_number_of_terms() << " training examples." << endl;
	
	LBFGSSolver solver;
	solver.maximum_iterations = 300;

	SolverResults results;
	solver.solve(function, &results);
	cout << results << endl << endl;

	// Show the resulting matrices.
	Eigen::Map<const Eigen::Matrix<double, n_hidden, n_input>>  W1_m(W1.data());
	Eigen::Map<const Eigen::Matrix<double, n_output, n_hidden>> W2_m(W2.data());
	cout << "Input --> hidden:" << endl << W1_m << endl << endl;
	cout << "Hidden --> output:" << endl << W2_m << endl << endl;

	auto test = training;
	test.push_back({0.5, 0.5});

	// Print a few classifications results.
	for (const auto& example : test) {
		input[0] = example[0];
		input[1] = example[1];
		output = neural_network_classify<double, n_input, n_hidden, n_output>(input, W1.data(), W2.data());
		cout << "(" << input[0] << ", " << input[1] << ") --> " << output[0] << endl;
	}
	
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

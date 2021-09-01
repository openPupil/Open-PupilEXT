// Include this function in a file defining the run_test
// template function.
//
// Petter Strandmark 2012
//

// ----------------------------------------------------------------------
//
// UCTP - Test Problems for Unconstrained Optimization. Report IMM-REP-2000-17,
// Department of Mathematical Modelling, DTU. November 2000.
//
// All problems except one is a nonlinear least-squares problem.
//
// #22
template<int n>
struct ExpAndSquares
{
	template<typename R>
	R operator()(const R* const x) const
	{
		using std::exp;

		R F = 0;
		R acc = 0;
		for (int j = 1; j <= n; ++j) {
			acc += x[j - 1];
		}
		F = exp( - acc);

		for (int j = 1; j <= n; ++j) {
			F += 0.5 * x[j - 1] * x[j - 1] * j * j;
		}

		return F;
	}
};
template<int n>
void test_exp_and_squares(double tol = 1e-6)
{
	std::vector<double> x(n, 0.0);
	run_test<ExpAndSquares<n>, n>(&x[0]);

	auto x_target = x;
	for (int j = 1; j <= n; ++j) {
		// Check that x_j = exp(-y) / j^2, where
		// y is the solution to (1 + ... + 1/n^2)exp(-y) = y
		double y = - std::log(x[j - 1] * j * j);
		INFO(x[j - 1]);
		INFO(y);
		double sum = 0;
		for (int i = 1; i <= n; ++i) {
			sum += 1.0 / (i * i);
		}
		CHECK((std::abs(sum * std::exp(-y) - y) / std::abs(y)) < tol);
	}
}

TEST(Solver, ExpAndSquares2)
{
	test_exp_and_squares<2>();
}

TEST(Solver, ExpAndSquares10)
{
	test_exp_and_squares<10>();
}

TEST(Solver, ExpAndSquares50)
{
	test_exp_and_squares<20>(1e-5);
}

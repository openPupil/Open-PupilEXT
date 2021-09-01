// Petter Strandmark 2013.
//
// This is a test of ConstrainedFunction with a known optimum.

#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <cstddef>

#include <spii/auto_diff_term.h>
#include <spii/constrained_function.h>
#include <spii/solver.h>

using namespace spii;

struct LinearObjective
{
	double c;
	LinearObjective(double c_)
		: c(c_)
	{  }

	template<typename R>
	R operator()(const R* const x) const
	{
		return c * x[0];
	}
};

// Constraint aTx ≤ b.
class LinearConstraintTerm
	: public spii::Term
{
public:
	std::vector<double> a;
	double b;

	LinearConstraintTerm(const std::vector<double>& a, double b)
	{
		this->a = a;
		this->b = b;
	}

	virtual int number_of_variables() const override
	{
		return int(a.size());
	}

	virtual int variable_dimension(int var) const override
	{
		return 1;
	}

	virtual double evaluate(double * const * const x) const override
	{
		double constraint = -b;
		for (size_t i = 0; i < a.size(); ++i) {
			constraint += a[i] * x[i][0];
		}
		return constraint;
	}

	virtual double evaluate(double * const * const x,
	                        std::vector<Eigen::VectorXd>* gradient) const override
	{
		double constraint = -b;
		for (size_t i = 0; i < a.size(); ++i) {
			constraint += a[i] * x[i][0];
		}

		for (size_t i = 0; i < a.size(); ++i) {
			(*gradient)[i](0) = a[i];
		}

		return constraint;
	}

	virtual double evaluate(double * const * const variables,
	                        std::vector<Eigen::VectorXd>* gradient,
	                        std::vector< std::vector<Eigen::MatrixXd> >* hessian
	                       ) const override
	{
		spii::check(false, "Hessians not supported.");
		return 0.0;
	}
};

int main_function()
{
	std::mt19937 prng(0);
	std::normal_distribution<double> normal;
	auto randn = std::bind(normal, prng);

	const int n = 100;

	// Variables.
	std::vector<double> x(n, 0.0);
	std::vector<double*> all_x_pointers;
	for (auto& single_x : x) {
		all_x_pointers.push_back(&single_x);
	}

	// Objective function.
	ConstrainedFunction f;

	// Generate random objective vector.
	std::vector<double> c(n);
	for (size_t i = 0; i < n; ++i) {
		c[i] = randn();
		f.add_term(std::make_shared<AutoDiffTerm<LinearObjective, 1>>(c[i]),
		           &x[i]);
	}

	// sum x_i <= 10
	double b = 10;
	std::vector<double> a1(n, 1.0);
	f.add_constraint_term(
		"sum x_i <= 0",
		std::make_shared<LinearConstraintTerm>(a1, b),
		all_x_pointers);

	//  sum x_i >= -10  <=>
	// -sum x_i <= 10
	std::vector<double> a2(n, -1.0);
	b = 10;
	f.add_constraint_term(
		"sum x_i >= -10",
		std::make_shared<LinearConstraintTerm>(a2, b),
		all_x_pointers);

	// Add barriers for individual scalars.
	//
	//		-100 <= x[i] <= 100
	//
	std::vector<double> a3(1, 1.0);
	auto individual_barrier_high = std::make_shared<LinearConstraintTerm>(a3, 100);
	std::vector<double> a4(1, -1.0);
	auto individual_barrier_low = std::make_shared<LinearConstraintTerm>(a4, 100);
	for (size_t i = 0; i < n; ++i) {
		f.add_constraint_term(to_string("x", i, " <= 100"), individual_barrier_high, &x[i]);
		f.add_constraint_term(to_string("-100 <= x", i), individual_barrier_low, &x[i]);
	}

	LBFGSSolver solver;
	SolverResults results;

	f.solve(solver, &results);
	check(f.is_feasible(), "Solution is not feasible.");

	// Check that all constraints are satisfied.
	double sum = 0;
	double eps = 1e-8;
	for (size_t i = 0; i < n; ++i) {
		spii_assert(-100-eps <= x[i] && x[i] <= 100+eps, "x[", i, "] = ", x[i]);
		sum += x[i];
		if (i < 10) {
			std::cout << "c[" << i << "] = " << c[i] << ", x[" << i << "] = " << x[i] << std::endl;
		}
	}
	spii_assert(-10-eps <= sum && sum <= 10+eps, "sum = ", sum);

	std::cout << "Solution to the linear programming problem is cTx = " << f.objective().evaluate() << '\n';

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

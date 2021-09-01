// Petter Strandmark 2012-2013.

#include <catch.hpp>
#include <spii/google_test_compatibility.h>

#include <spii/auto_diff_term.h>
#include <spii/function.h>
#include <spii/interval_term.h>
#include <spii/transformations.h>

using namespace spii;

TEST(Function, get_number_of_scalars)
{
	Function f;
	double x[5];
	double y[4];
	double z[2];
	f.add_variable(x, 5);
	f.add_variable(y, 4);
	f.add_variable(z, 2);
	EXPECT_EQ(f.get_number_of_scalars(), 11);
}

TEST(Function, added_same_variable_multiple_times)
{
	Function f;
	double x[5];
	f.add_variable(x, 5);
	f.add_variable(x, 5); // No-op.
	EXPECT_EQ(f.get_number_of_scalars(), 5);
	EXPECT_THROW(f.add_variable(x, 4), std::runtime_error);
}

class Term1
{
public:
	template<typename R>
	R operator()(const R* const x) const
	{
		return sin(x[0]) + cos(x[1]) + R(1.4)*x[0]*x[1] + R(1.0);
	}
};

class Term2
{
public:
	template<typename R>
	R operator()(const R* const x, const R* const y) const
	{
		return log(x[0]) + 3.0 * log(y[0]);
	}
};

TEST(Function, variable_not_found_is_added)
{
	Function f;
	double x[2] = {0};
	f.add_term<AutoDiffTerm<Term1, 2>>(x);
	CHECK(f.get_number_of_terms() == 1);
	CHECK(f.get_number_of_variables() == 1);
	CHECK(f.get_number_of_scalars() == 2);
}

TEST(Function, term_variable_mismatch)
{
	Function f;
	double x[5] = {0};
	f.add_variable(x, 5);
	EXPECT_THROW((f.add_term<AutoDiffTerm<Term1, 4>>(x)), std::logic_error);
}

class DestructorTerm :
	public SizedTerm<1>
{
public:
	DestructorTerm(int* counter)
	{
		this->counter = counter;
	}

	~DestructorTerm()
	{
		(*counter)++;
	}

	virtual double evaluate(double * const * const variables) const
	{
		return 0;
	}

	virtual double evaluate(double * const * const variables,
	                        std::vector<Eigen::VectorXd>* gradient) const
	{
		return 0;
	}

	virtual double evaluate(double * const * const variables,
	                        std::vector<Eigen::VectorXd>* gradient,
	                        std::vector< std::vector<Eigen::MatrixXd> >* hessian) const
	{
		return 0;
	}

private:
	int* counter;
};

TEST(Function, calls_term_destructor)
{
	Function* function = new Function;
	double x[1];
	function->add_variable(x, 1);

	int counter1 = 0;
	int counter2 = 0;

	{
		auto term1 = std::make_shared<DestructorTerm>(&counter1);
		auto term2 = std::make_shared<DestructorTerm>(&counter2);

		function->add_term(term1, x);
		function->add_term(term1, x);
		function->add_term(term2, x);
	}

	EXPECT_EQ(counter1, 0);
	EXPECT_EQ(counter2, 0);
	delete function;
	EXPECT_EQ(counter1, 1);
	EXPECT_EQ(counter2, 1);
}

TEST(Function, copy_constructor)
{
	Function* function = new Function;
	double x[1];
	function->add_variable(x, 1);

	int counter1 = 0;
	int counter2 = 0;

	{
		auto term1 = std::make_shared<DestructorTerm>(&counter1);
		auto term2 = std::make_shared<DestructorTerm>(&counter2);

		function->add_term(term1, x);
		function->add_term(term1, x);
		function->add_term(term2, x);
	}

	EXPECT_EQ(counter1, 0);
	EXPECT_EQ(counter2, 0);

	{
		Function function_copy = *function;
		EXPECT_EQ(counter1, 0);
		EXPECT_EQ(counter2, 0);
	}

	EXPECT_EQ(counter1, 0);
	EXPECT_EQ(counter2, 0);

	delete function;

	EXPECT_EQ(counter1, 1);
	EXPECT_EQ(counter2, 1);
}

class DestructorChange
{
public:
	DestructorChange(int* counter)
	{
		this->counter = counter;
	}

	~DestructorChange()
	{
		(*counter)++;
	}

	template<typename R>
	void t_to_x(R* x, const R* t) const { }

	template<typename R>
	void x_to_t(R* t, const R* x) const { }

	int x_dimension() const
	{
		return 1;
	}

	int t_dimension() const
	{
		return 1;
	}
private:
	int* counter;
};

TEST(Function, calls_variable_change_destructor)
{
	Function* function = new Function;
	double x[1];
	int counter = 0;
	function->add_variable_with_change<DestructorChange>(x, 1, &counter);

	EXPECT_EQ(counter, 0);
	delete function;
	EXPECT_EQ(counter, 1);
}

TEST(Function, evaluate)
{

	double x[2] = {1.0, 2.0};
	double y[1] = {3.0};
	double z[1] = {4.0};

	Function f;
	f.add_variable(x, 2);
	f.add_variable(y, 1);
	f.add_variable(z, 1);

	f.add_term(std::make_shared<AutoDiffTerm<Term1, 2>>(), x);
	f.add_term(std::make_shared<AutoDiffTerm<Term2, 1, 1>>(), y, z);

	double fval = f.evaluate();
	EXPECT_DOUBLE_EQ(fval, sin(x[0]) + cos(x[1]) + 1.4 * x[0]*x[1] + 1.0 +
	                       log(y[0]) + 3.0 * log(z[0]));

	for (int b1 = 0; b1 <= 1; b1++) {
	for (int b2 = 0; b2 <= 1; b2++) {
	for (int b3 = 0; b3 <= 1; b3++) {
		f.set_constant(x, b1 == 0);
		f.set_constant(y, b2 == 0);
		f.set_constant(z, b3 == 0);
		EXPECT_EQ(f.get_number_of_scalars(), b1*2 + b2 + b3);
		double fval = f.evaluate();
		EXPECT_DOUBLE_EQ(fval, sin(x[0]) + cos(x[1]) + 1.4 * x[0]*x[1] + 1.0 +
		                 log(y[0]) + 3.0 * log(z[0]));
	}}}
}

TEST(Function, add_constant)
{
	double x[2] = {1.0, 2.0};
	double y[1] = {3.0};
	double z[1] = {4.0};

	Function f;
	f.add_variable(x, 2);
	f.add_variable(y, 1);
	f.add_variable(z, 1);

	f.add_term<AutoDiffTerm<Term1, 2>>(x);
	f.add_term<AutoDiffTerm<Term2, 1, 1>>(y, z);

	double fval = f.evaluate();
	f += 1.0;
	CHECK(f.evaluate() == fval + 1.0);
	f += 10.0;
	CHECK(f.evaluate() == fval + 1.0 + 10.0);
}

TEST(Function, add_functions)
{
	double x[2] = { 1.0, 2.0 };
	double y[1] = { 3.0 };
	double z[1] = { 4.0 };

	Function f;
	f.add_variable(x, 2);
	f.add_variable(y, 1);
	f.add_variable(z, 1);

	f.add_term<AutoDiffTerm<Term1, 2>>(x);
	f.add_term<AutoDiffTerm<Term2, 1, 1>>(y, z);
	f += 7.0;

	Function f2;
	CHECK(f2.evaluate() == 0.0);

	for (int i = 1; i <= 20; ++i) {
		f2 += f;
		CHECK(Approx(f2.evaluate()) == i * f.evaluate());
	}
}

TEST(Function, evaluate_x)
{

	double x[2] = {1.0, 2.0};
	double y[1] = {3.0};
	double z[1] = {4.0};

	Function f;
	f.add_variable(x, 2);
	f.add_variable(y, 1);
	f.add_variable(z, 1);

	f.add_term<AutoDiffTerm<Term1, 2>>(x);
	f.add_term<AutoDiffTerm<Term2, 1, 1>>(y, z);

	Eigen::VectorXd xg(4);
	xg[0] = 6.0;
	xg[1] = 7.0;
	xg[2] = 8.0;
	xg[3] = 9.0;

	double fval = f.evaluate(xg);
	EXPECT_DOUBLE_EQ(fval, sin(xg[0]) + cos(xg[1]) + 1.4 * xg[0]*xg[1] + 1.0 +
	                       log(xg[2]) + 3.0 * log(xg[3]));
}

TEST(Function, copy_and_assignment)
{
	double x[2] = {1.0, 2.0};
	double y[1] = {3.0};
	double z[1] = {4.0};

	int counter = 0;
	auto f1 = new Function;
	auto f2 = new Function;

	{
		auto destructor_term = std::make_shared<DestructorTerm>(&counter);
	
		f1->add_variable(x, 2);
		f1->add_variable(y, 1);
		f1->add_variable(z, 1);
		f1->add_term<AutoDiffTerm<Term1, 2>>(x);
		f1->add_term<AutoDiffTerm<Term2, 1, 1>>(y, z);
		f1->add_term(destructor_term, y);
		REQUIRE(counter == 0);

		f2->add_variable(x, 2);
		f2->add_variable(y, 1);
		f2->add_variable(z, 1);
		f2->add_term(destructor_term, y);
		f2->add_term(destructor_term, z);
		REQUIRE(counter == 0);
	}
	REQUIRE(counter == 0);

	auto f3 = new Function{*f1};
	REQUIRE(counter == 0);

	auto f4 = new Function{*f2};
	REQUIRE(counter == 0);

	CHECK(f3->get_number_of_scalars() == f1->get_number_of_scalars());
	CHECK(f3->get_number_of_terms() == f1->get_number_of_terms());
	CHECK(f3->evaluate() == f1->evaluate());
	CHECK(f4->evaluate() == f2->evaluate());

	{
		Function tmp = *f3;
		*f3 = *f4;
		*f4 = tmp;
		REQUIRE(counter == 0);
	}
	REQUIRE(counter == 0);

	CHECK(f4->evaluate() == f1->evaluate());
	CHECK(f2->evaluate() == f2->evaluate());

	delete f4;
	REQUIRE(counter == 0);
	delete f2;
	REQUIRE(counter == 0);
	delete f3;
	REQUIRE(counter == 0);
	delete f1;
	REQUIRE(counter == 1);
}


TEST(Function, evaluate_gradient)
{

	double x[2] = {1.0, 2.0};
	double y[1] = {3.0};
	double z[1] = {4.0};

	Function f;
	f.add_variable(x, 2);
	f.add_variable(y, 1);
	f.add_variable(z, 1);

	f.add_term(std::make_shared<AutoDiffTerm<Term1, 2>>(), x);
	f.add_term(std::make_shared<AutoDiffTerm<Term1, 2>>(), x);  // Add term twice for testing.
	f.add_term(std::make_shared<AutoDiffTerm<Term2, 1, 1>>(), y, z);

	Eigen::VectorXd xg(4);
	xg[0] = 6.0;
	xg[1] = 7.0;
	xg[2] = 8.0;
	xg[3] = 9.0;

	Eigen::VectorXd gradient;
	Eigen::MatrixXd hessian;

	auto fval_ref = f.evaluate(xg, &gradient, &hessian);
	EXPECT_EQ(gradient.size(), 4);

	// Check gradient values.
	// x term was added twice, hence 2.0.
	EXPECT_DOUBLE_EQ(gradient[0], 2.0 * (cos(xg[0]) + 1.4 * xg[1]));
	EXPECT_DOUBLE_EQ(gradient[1], 2.0 * (-sin(xg[1]) + 1.4 * xg[0]));
	EXPECT_DOUBLE_EQ(gradient[2], 1.0 / xg[2]);
	EXPECT_DOUBLE_EQ(gradient[3], 3.0 / xg[3]);

	f.copy_global_to_user(xg);

	for (int b1 = 0; b1 <= 1; b1++) {
	for (int b2 = 0; b2 <= 1; b2++) {
	for (int b3 = 0; b3 <= 1; b3++) {
		f.set_constant(x, b1 == 0);
		f.set_constant(y, b2 == 0);
		f.set_constant(z, b3 == 0);
		Eigen::VectorXd xg(2*b1 + b2 + b3);
		f.copy_user_to_global(&xg);

		Eigen::VectorXd gradient;
		Eigen::MatrixXd hessian;
		auto fval = f.evaluate(xg, &gradient, &hessian);
		EXPECT_EQ(fval, fval_ref);
		EXPECT_EQ(gradient.size(), 2*b1 + b2 + b3);

		if (b1 == 1) {
			auto ind = f.get_variable_global_index(x);
			EXPECT_EQ(xg[ind+0], x[0]);
			EXPECT_EQ(xg[ind+1], x[1]);
			EXPECT_DOUBLE_EQ(gradient[ind+0], 2.0 * (cos(xg[ind+0]) + 1.4 * xg[ind+1]));
			EXPECT_DOUBLE_EQ(gradient[ind+1], 2.0 * (-sin(xg[ind+1]) + 1.4 * xg[ind+0]));
		}
		if (b2 == 1) {
			auto ind = f.get_variable_global_index(y);
			EXPECT_EQ(xg[ind], y[0]);
			EXPECT_DOUBLE_EQ(gradient[ind], 1.0 / xg[ind]);
		}
		if (b3 == 1) {
			auto ind = f.get_variable_global_index(z);
			EXPECT_EQ(xg[ind], z[0]);
			EXPECT_DOUBLE_EQ(gradient[ind], 3.0 / xg[ind]);
		}
	}}}
}

class Single3
{
public:
	template<typename R>
	R operator()(const R* const x) const
	{
		return 123.4 * x[0]*x[0] + 7.0 * sin(x[1]) + 2.0 * x[0]*x[1] + 3.0 * x[2]*x[2];
	}
};

class Single2
{
public:
	template<typename R>
	R operator()(const R* const x) const
	{
		return 5.0 * x[0]*x[0] + 6.0 * x[0]*x[1] + 7.0 * x[1]*x[1];
	}
};

class Mixed3_2
{
public:
	template<typename R>
	R operator()(const R* const x, const R* const y) const
	{
		return    9.0 * x[0]*y[0] + 10.0 * x[0]*y[1]
		       + 11.0 * x[1]*y[0] + 12.0 * x[1]*y[1]
			   + 13.0 * x[2]*y[0] + 14.0 * cos(x[2]*y[1]);
	}
};

TEST(Function, evaluate_hessian)
{

	double x[3] = {1.0, 2.0, 3.0};
	double y[2] = {3.0, 4.0};

	Function f;
	f.add_variable(x, 3);
	f.add_variable(y, 2);

	f.add_term(std::make_shared<AutoDiffTerm<Single3, 3>>(), x);
	f.add_term(std::make_shared<AutoDiffTerm<Single2, 2>>(), y);
	f.add_term(std::make_shared<AutoDiffTerm<Mixed3_2, 3, 2>>(), x, y);

	Eigen::VectorXd xg(5);
	xg[0] = 6.0; // x[0]
	xg[1] = 7.0; // x[1]
	xg[2] = 8.0; // x[2]
	xg[3] = 9.0; // y[0]
	xg[4] = 1.0; // y[1]

	Eigen::VectorXd gradient;
	Eigen::MatrixXd hessian;

	f.evaluate(xg, &gradient, &hessian);
	ASSERT_EQ(hessian.rows(), 5);
	ASSERT_EQ(hessian.cols(), 5);

	// Check the x part of hessian.
	EXPECT_DOUBLE_EQ(hessian(0,0), 2.0 * 123.4);
	EXPECT_DOUBLE_EQ(hessian(1,1), - 7.0 * sin(xg[1]));
	EXPECT_DOUBLE_EQ(hessian(2,2), 2.0 * 3.0
	                               - 14.0 * xg[4] * xg[4] * cos(xg[2]*xg[4]));

	EXPECT_DOUBLE_EQ(hessian(0,1), 2.0);
	EXPECT_DOUBLE_EQ(hessian(1,0), 2.0);
	EXPECT_DOUBLE_EQ(hessian(0,2), 0.0);
	EXPECT_DOUBLE_EQ(hessian(2,0), 0.0);
	EXPECT_DOUBLE_EQ(hessian(1,2), 0.0);
	EXPECT_DOUBLE_EQ(hessian(2,1), 0.0);

	// Check the y part of hessian.
	EXPECT_DOUBLE_EQ(hessian(3,3), 2.0 * 5.0);
	EXPECT_DOUBLE_EQ(hessian(4,4), 2.0 * 7.0
		                           - 14.0 * xg[2] * xg[2] * cos(xg[2]*xg[4]));
	EXPECT_DOUBLE_EQ(hessian(3,4), 6.0);
	EXPECT_DOUBLE_EQ(hessian(4,3), 6.0);

	// Check the x-y part of hessian.
	EXPECT_DOUBLE_EQ(hessian(0,3),  9.0);
	EXPECT_DOUBLE_EQ(hessian(3,0),  9.0);
	EXPECT_DOUBLE_EQ(hessian(0,4), 10.0);
	EXPECT_DOUBLE_EQ(hessian(4,0), 10.0);
	EXPECT_DOUBLE_EQ(hessian(1,3), 11.0);
	EXPECT_DOUBLE_EQ(hessian(3,1), 11.0);
	EXPECT_DOUBLE_EQ(hessian(1,4), 12.0);
	EXPECT_DOUBLE_EQ(hessian(4,1), 12.0);
	EXPECT_DOUBLE_EQ(hessian(2,3), 13.0);
	EXPECT_DOUBLE_EQ(hessian(3,2), 13.0);
	EXPECT_DOUBLE_EQ(hessian(2,4), - 14.0 * (sin(xg[2]*xg[4]) + xg[2] * xg[4] * cos(xg[2]*xg[4])));
	EXPECT_DOUBLE_EQ(hessian(4,2), - 14.0 * (sin(xg[2]*xg[4]) + xg[2] * xg[4] * cos(xg[2]*xg[4])));


	f.copy_global_to_user(xg);

	for (int b1 = 0; b1 <= 1; b1++) {
	for (int b2 = 0; b2 <= 1; b2++) {
		f.set_constant(x, b1 == 0);
		f.set_constant(y, b2 == 0);
		Eigen::VectorXd xg(3*b1 + 2*b2);
		f.copy_user_to_global(&xg);

		Eigen::VectorXd gradient;
		Eigen::MatrixXd hessian;
		f.evaluate(xg, &gradient, &hessian);

		EXPECT_EQ(hessian.rows(), 3*b1 + 2*b2);
		EXPECT_EQ(hessian.cols(), 3*b1 + 2*b2);

		if (b1 == 1) {
			auto ind = f.get_variable_global_index(x);
			auto ind0 = ind;
			auto ind1 = ind + 1;
			auto ind2 = ind + 2;
			EXPECT_DOUBLE_EQ(hessian(ind0,ind1), 2.0);
			EXPECT_DOUBLE_EQ(hessian(ind1,ind0), 2.0);
			EXPECT_DOUBLE_EQ(hessian(ind0,ind2), 0.0);
			EXPECT_DOUBLE_EQ(hessian(ind2,ind0), 0.0);
			EXPECT_DOUBLE_EQ(hessian(ind1,ind2), 0.0);
			EXPECT_DOUBLE_EQ(hessian(ind2,ind1), 0.0);
		}
		if (b2 == 1) {
			auto ind = f.get_variable_global_index(y);
			auto ind0 = ind;
			auto ind1 = ind + 1;
			EXPECT_DOUBLE_EQ(hessian(ind0,ind0), 2.0 * 5.0);
			EXPECT_DOUBLE_EQ(hessian(ind1,ind1),
				2.0 * 7.0 - 14.0 * x[2] * x[2] * cos(x[2]*y[1]));
			EXPECT_DOUBLE_EQ(hessian(ind0,ind1), 6.0);
			EXPECT_DOUBLE_EQ(hessian(ind1,ind0), 6.0);
		}
	}}
}


TEST(Function, evaluation_count)
{

	double x[3] = {1.0, 2.0, 3.0};
	double y[2] = {3.0, 4.0};

	Function f;
	f.add_variable(x, 3);
	f.add_variable(y, 2);

	f.add_term(std::make_shared<AutoDiffTerm<Single3, 3>>(), x);
	f.add_term(std::make_shared<AutoDiffTerm<Single2, 2>>(), y);
	f.add_term(std::make_shared<AutoDiffTerm<Mixed3_2, 3, 2>>(), x, y);

	Eigen::VectorXd xg(5);
	xg.setZero();
	Eigen::VectorXd gradient;
	Eigen::MatrixXd hessian;
	Eigen::SparseMatrix<double> sparse_hessian;
	f.create_sparse_hessian(&sparse_hessian);

	f.evaluate();
	f.evaluate();
	f.evaluate(xg);
	f.evaluate(xg, &gradient, &hessian);

	EXPECT_EQ(f.evaluations_without_gradient, 3);
	EXPECT_EQ(f.evaluations_with_gradient, 1);
	f.evaluate(xg, &gradient, &hessian);
	f.evaluate(xg, &gradient, &sparse_hessian);
	EXPECT_EQ(f.evaluations_with_gradient, 3);
}

//
//	x_i = exp(t_i)
//  t_i = log(x_i)
//
template<int dimension>
class ExpTransform
{
public:
	template<typename R>
	void t_to_x(R* x, const R* t) const
	{
		using std::exp;

		for (size_t i = 0; i < dimension; ++i) {
			x[i] = exp(t[i]);
		}
	}

	template<typename R>
	void x_to_t(R* t, const R* x) const
	{
		using std::log;

		for (size_t i = 0; i < dimension; ++i) {
			t[i] = log(x[i]);
		}
	}

	int x_dimension() const
	{
		return dimension;
	}

	int t_dimension() const
	{
		return dimension;
	}
};

TEST(Function, Parametrization_2_to_2)
{
	Function f1, f2;
	double x[2];
	f1.add_variable(x, 2);
	f2.add_variable_with_change<ExpTransform<2>>(x, 2);

	f1.add_term(std::make_shared<AutoDiffTerm<Term1, 2>>(), x);
	f2.add_term(std::make_shared<AutoDiffTerm<Term1, 2>>(), x);

	EXPECT_EQ(f1.get_number_of_scalars(), 2);
	EXPECT_EQ(f2.get_number_of_scalars(), 2);

	for (x[0] = 0.1; x[0] <= 10.0; x[0] += 0.1) {
		x[1] = x[0] / 2.0 + 0.1;
		EXPECT_NEAR(f1.evaluate(), f2.evaluate(), 1e-12);
	}

	// Term1 is
	// f(x1, x2) = sin(x1) + cos(x2) + 1.4 * x1*x2 + 1.0
	//
	// f(t1, t2) = sin(exp(t1)) + cos(exp(t2)) + 1.4 * exp(t1)*exp(t2) + 1.0
	Eigen::VectorXd x_vec(2);
	x_vec[0] = 3.0;
	x_vec[1] = 4.0;
	Eigen::VectorXd t(2);
	t[0] = std::log(x_vec[0]);
	t[1] = std::log(x_vec[1]);
	Eigen::VectorXd x_gradient;
	Eigen::VectorXd t_gradient;
	double f1_val = f1.evaluate(x_vec, &x_gradient);
	double f2_val = f2.evaluate(t, &t_gradient);

	// The function values must match.
	EXPECT_NEAR(f1_val, f2_val, 1e-12);

	// The gradient of f1 is straight-forward.
	EXPECT_NEAR(x_gradient[0],  cos(x_vec[0]) + 1.4 * x_vec[1], 1e-12);
	EXPECT_NEAR(x_gradient[1], -sin(x_vec[1]) + 1.4 * x_vec[0], 1e-12);

	// The gradient of f2 is in the transformed space.
	EXPECT_NEAR(t_gradient[0],  cos(exp(t[0]))*exp(t[0]) + 1.4 * exp(t[0]) * exp(t[1]), 1e-12);
	EXPECT_NEAR(t_gradient[1], -sin(exp(t[1]))*exp(t[1]) + 1.4 * exp(t[0]) * exp(t[1]), 1e-12);
}

TEST(Function, Parametrization_1_1_to_1_1)
{
	Function f1, f2;
	double x[1];
	double y[1];
	f1.add_variable(x, 1);
	f1.add_variable(y, 1);
	f2.add_variable_with_change<ExpTransform<1>>(x, 1);
	f2.add_variable_with_change<ExpTransform<1>>(y, 1);

	f1.add_term(std::make_shared<AutoDiffTerm<Term2, 1, 1>>(), x, y);
	f2.add_term(std::make_shared<AutoDiffTerm<Term2, 1, 1>>(), x, y);

	EXPECT_EQ(f1.get_number_of_scalars(), 2);
	EXPECT_EQ(f2.get_number_of_scalars(), 2);

	for (x[0] = 0.1; x[0] <= 10.0; x[0] += 0.1) {
		y[0] = x[0] / 2.0 + 0.1;
		EXPECT_NEAR(f1.evaluate(), f2.evaluate(), 1e-12);
	}

	// Term2 is
	// f(x, y) = log(x) + 3.0 * log(y);
	//
	// f(s, t) = s + 3.0 * t
	Eigen::VectorXd xy(2);
	xy[0] = 3.0;
	xy[1] = 4.0;
	Eigen::VectorXd st(2);
	st[0] = std::log(xy[0]);
	st[1] = std::log(xy[1]);
	Eigen::VectorXd xy_gradient;
	Eigen::VectorXd st_gradient;
	double f1_val = f1.evaluate(xy, &xy_gradient);
	double f2_val = f2.evaluate(st, &st_gradient);

	// The function values must match.
	EXPECT_NEAR(f1_val, f2_val, 1e-12);

	// The gradient of f1 is straight-forward.
	EXPECT_NEAR(xy_gradient[0], 1.0 / xy[0], 1e-12);
	EXPECT_NEAR(xy_gradient[1], 3.0 / xy[1], 1e-12);

	// The gradient of f2 is in the transformed space.
	EXPECT_NEAR(st_gradient[0],  1.0, 1e-12);
	EXPECT_NEAR(st_gradient[1],  3.0, 1e-12);
}

class Circle
{
public:
	template<typename R>
	void t_to_x(R* x, const R* t) const
	{
		using std::cos;
		using std::cin;

		x[0] = cos(t[0]);
		x[1] = sin(t[0]);
	}

	template<typename R>
	void x_to_t(R* t, const R* x) const
	{
		using std::atan2;

		t[0] = atan2(x[1], x[0]);
	}

	int x_dimension() const
	{
		return 2;
	}

	int t_dimension() const
	{
		return 1;
	}
};

TEST(Function, Parametrization_2_to_1)
{
	Function f1, f2;
	double x[2];
	f1.add_variable(x, 2);
	f2.add_variable_with_change<Circle>(x, 2);

	f1.add_term(std::make_shared<AutoDiffTerm<Term1, 2>>(), x);
	f2.add_term(std::make_shared<AutoDiffTerm<Term1, 2>>(), x);

	EXPECT_EQ(f1.get_number_of_scalars(), 2);
	EXPECT_EQ(f2.get_number_of_scalars(), 1);

	for (double theta = 0.0; theta <= 6.0; theta += 0.5) {
		x[0] = std::cos(theta);
		x[1] = std::sin(theta);
		EXPECT_NEAR(f1.evaluate(), f2.evaluate(), 1e-12);
	}
}

class ThrowsRuntimeError
{
public:
	template<typename R>
	R operator()(const R* const x) const
	{
		throw std::runtime_error("Error message");
		return 0;
	}
};

class ThrowsCString
{
public:
	template<typename R>
	R operator()(const R* const x) const
	{
		throw "Error";
		return 0;
	}
};

TEST(Function, rethrows_error)
{
	double x = 0;

	Function f1;
	f1.add_variable(&x, 1);
	for (int i = 1; i < 40; ++i) {
		f1.add_term(std::make_shared<IntervalTerm<ThrowsRuntimeError, 1>>(),
		            &x);
	}
	Eigen::VectorXd x_vec(1);
	x_vec[0] = x;
	Eigen::VectorXd g(1);
	Eigen::MatrixXd H(1, 1);
	Eigen::SparseMatrix<double> H_sparse;
	IntervalVector x_interval(1, 0.0);
	EXPECT_THROW(f1.evaluate(), std::runtime_error);
	EXPECT_THROW(f1.evaluate(x_vec), std::runtime_error);
	EXPECT_THROW(f1.evaluate(x_vec, &g), std::runtime_error);
	EXPECT_THROW(f1.evaluate(x_vec, &g, &H), std::runtime_error);
	EXPECT_THROW(f1.evaluate(x_vec, &g, &H_sparse), std::runtime_error);
	EXPECT_THROW(f1.evaluate(x_interval), std::runtime_error);

	Function f2;
	f2.add_variable(&x, 1);
	for (int i = 1; i < 40; ++i) {
		f2.add_term(std::make_shared<IntervalTerm<ThrowsCString, 1>>(),
		            &x);
	}
	EXPECT_THROW(f2.evaluate(), const char*);
	EXPECT_THROW(f2.evaluate(x_vec), const char*);
	EXPECT_THROW(f2.evaluate(x_vec, &g), const char*);
	EXPECT_THROW(f2.evaluate(x_vec, &g, &H), const char*);
	EXPECT_THROW(f2.evaluate(x_vec, &g, &H_sparse), const char*);
	EXPECT_THROW(f2.evaluate(x_interval), const char*);
}

struct SimplePolynomial
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return  x[0]*x[0]*x[0]*x[0] - 20.0*x[0]*x[0]*x[0] + 150.0*x[0]*x[0] - 500.0*x[0] + 625.0;
	}
};

TEST(Function, evaluate_interval)
{
	using namespace spii;

	double x = 2.0;
	IntervalVector x_interval;
	x_interval.push_back(Interval<double>(2.0, 2.0));

	Function f;
	f.add_variable(&x, 1);
	f.add_term(std::make_shared<IntervalTerm<SimplePolynomial, 1>>(),
	           &x);

	EXPECT_DOUBLE_EQ(f.evaluate(), 81.0);

	auto result = f.evaluate(x_interval);
	Interval<double> expected(81, 81);
	EXPECT_DOUBLE_EQ(result.get_lower(), expected.get_lower());
	EXPECT_DOUBLE_EQ(result.get_upper(), expected.get_upper());
}

TEST_CASE("variables_overlap_1")
{
	Function f;
	double x[5];
	f.add_variable(&x[0], 5);
	EXPECT_THROW(f.add_variable(&x[2], 1), std::exception);
}

TEST_CASE("variables_overlap_2")
{
	Function f;
	double x[5];
	f.add_variable(&x[2], 1);
	EXPECT_THROW(f.add_variable(&x[0], 5), std::exception);
}

TEST_CASE("variables_overlap_3")
{
	Function f;
	double x[5];
	f.add_variable(&x[3], 1);
	f.add_variable(&x[4], 2);
	f.add_variable(&x[0], 1);
	f.add_variable(&x[1], 1);
	EXPECT_THROW(f.add_variable(&x[2], 2), std::exception);
}

TEST_CASE("begin_end_terms")
{
	Function f;
	double xx[2] = {0, 0};
	double x[1] = {0};
	double y[1] = {0};
	f.add_variable(xx, 2);
	f.add_variable(x, 1);
	f.add_variable(y, 1);

	auto term1 = make_differentiable<2>(Term1{});
	auto term2 = make_differentiable<1, 1>(Term2{});
	f.add_term(term1, xx);
	f.add_term(term2, x, y);
	
	int n_iterations = 0;
	for (const auto& added_term: f.terms()) {
		n_iterations++;
		CHECK((added_term.term == term1 || added_term.term == term2));
	}
	CHECK(n_iterations == 2);
}

TEST_CASE("invalid_user_data")
{
	Function f;
	double xx[2] = {0, 0};
	double x[1] = {0};
	double y[1] = {0};
	f.add_variable(xx, 2);
	f.add_variable(x, 1);
	f.add_variable(y, 1);

	auto term1 = make_differentiable<2>(Term1{});
	auto term2 = make_differentiable<1, 1>(Term2{});
	f.add_term(term1, xx);
	f.add_term(term2, x, y);

	Eigen::VectorXd eigen_vector;

	CHECK_NOTHROW(f.evaluate());
	CHECK_NOTHROW(f.copy_user_to_global(&eigen_vector));

	x[0] = std::numeric_limits<double>::quiet_NaN();
	CHECK_THROWS(f.evaluate());
	CHECK_THROWS(f.copy_user_to_global(&eigen_vector));
	x[0] = 0;
	y[0] = std::numeric_limits<double>::infinity();
	CHECK_THROWS(f.evaluate());
	CHECK_THROWS(f.copy_user_to_global(&eigen_vector));
}

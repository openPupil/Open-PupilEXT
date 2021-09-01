// Petter Strandmark 2012.
#include <sstream>

#include <catch.hpp>

#include <spii/auto_diff_term.h>
#include <spii/term.h>

using namespace fadbad;
using namespace spii;

class Func
{
	public:
	template <class T>
	T operator()(const T* x) const
	{
		T z=sqrt(x[0]);
		return x[1]*z+sin(z);
	}
};

class DFunc
{
	public:
	template <class T>
	T operator()(
		T& o_dfdx, T& o_dfdy,
		const T& i_x, const T& i_y)
	{
		F<T, 2> x(i_x), y(i_y);
		x.diff(0);
		y.diff(1);
		Func func;
		F<T, 2> f(func(x,y));
		o_dfdx = f.d(0);
		o_dfdy = f.d(1);

		return f.x();       // Return function value
	}
};

template<typename Functor>
class DDFunc
{
	public:
	template <class T>
	T operator()(
		T& o_dfdxdx, T& o_dfdxdy,
		T& o_dfdydx, T& o_dfdydy,
		T& o_dfdx, T& o_dfdy,
		const T& i_x, const T& i_y)
	{
		Functor func;
		F<T, 2> x[2]  = {i_x, i_y};
		x[0].diff(0);
		x[1].diff(1);
		F<T, 2> df[2];
		F<T, 2> f(differentiate_functor<Functor, F<T, 2>, 2>(func, x, df));   // Evaluate function and derivatives
		o_dfdx = df[0].x();
		o_dfdy = df[1].x();
		o_dfdxdx = df[0].d(0);
		o_dfdxdy = df[0].d(1);
		o_dfdydx = df[1].d(0);
		o_dfdydy = df[1].d(1);

		return f.x();
	}

};

TEST_CASE("FADBAD/differentiate_functor", "")
{
	using namespace std;
	double f,dfdx,dfdy,
	       dfdxdx,dfdxdy,
	       dfdydx,dfdydy;
	double x=1.3;
	double y=2;
	f=DDFunc<Func>()(dfdxdx,dfdxdy,
	                 dfdydx,dfdydy,
	                 dfdx,dfdy,x,y);  // Evaluate function and derivatives

	// Check all derivatives.
	CHECK(Approx(f) == y * sqrt(x) + sin(sqrt(x)));
	CHECK(Approx(dfdx) == (y + cos(sqrt(x))) / (2.0*sqrt(x)));
	CHECK(Approx(dfdy) == sqrt(x));
	CHECK(Approx(dfdxdx) == -(y + cos(sqrt(x)) + sqrt(x)*sin(sqrt(x)))/(4*pow(x,3.0/2.0)));
	CHECK(Approx(dfdxdy) == 1.0 / (2.0*sqrt(x)));
	CHECK(Approx(dfdydx) == 1.0 / (2.0*sqrt(x)));
	CHECK(Approx(dfdydy) == 0.0);
}

class MyTerm: public SizedTerm<2, 3>
{
public:
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
};

TEST_CASE("SizedTerm/number_of_variables", "")
{
	MyTerm term;
	CHECK(term.number_of_variables() == 2);
}

TEST_CASE("SizedTerm/variable_dimension", "")
{
	MyTerm term;
	CHECK(term.variable_dimension(0) == 2);
	CHECK(term.variable_dimension(1) == 3);
}

class DestructorFunctor
{
public:
	DestructorFunctor(int* counter)
	{
		this->counter = counter;
	}

	~DestructorFunctor()
	{
		(*counter)++;
	}

	template<typename R>
	R operator()(const R* const x) const
	{
		return 0.0;
	}

private:
	int* counter;
};

TEST_CASE("AutoDiffTerm/calls_functor_destructor", "")
{
	int counter = 0;
	Term* term = new AutoDiffTerm<DestructorFunctor, 1>(&counter);
	delete term;
	CHECK(counter == 1);
}

class MyFunctor1
{
public:
	template<typename R>
	R operator()(const R* const x) const
	{
		return sin(x[0]) + cos(x[1]) + R(1.4)*x[0]*x[1] + R(1.0);
	}
};

TEST_CASE("AutoDiffTerm/make_differentiable")
{
	auto term1 = make_differentiable<1>(MyFunctor1{});
}

TEST_CASE("AutoDiffTerm/MyFunctor1", "")
{
	AutoDiffTerm<MyFunctor1, 2> term;

	double x[2] = {1.0, 3.0};
	std::vector<double*> variables;
	variables.push_back(x);

	std::vector<Eigen::VectorXd> gradient;
	gradient.push_back(Eigen::VectorXd(2));

	std::vector< std::vector<Eigen::MatrixXd> > hessian(1);
	hessian[0].resize(1);
	hessian[0][0].resize(2,2);

	double value  = term.evaluate(&variables[0], &gradient);
	double value2 = term.evaluate(&variables[0]);

	// The two values must agree.
	CHECK(Approx(value) == value2);

	// Test function value
	CHECK(Approx(value) == sin(x[0]) + cos(x[1]) + 1.4*x[0]*x[1] + 1.0);

	// Test gradient
	CHECK(Approx(gradient[0](0)) ==  cos(x[0]) + 1.4*x[1]);
	CHECK(Approx(gradient[0](1)) == -sin(x[1]) + 1.4*x[0]);

	for (auto& g : gradient) {
		g.setZero();
	}
	double value3 = term.evaluate(&variables[0], &gradient, &hessian);
	CHECK(Approx(value3) == value2);

	// Test gradient
	CHECK(Approx(gradient[0](0)) == cos(x[0]) + 1.4*x[1]);
	CHECK(Approx(gradient[0](1)) == -sin(x[1]) + 1.4*x[0]);

	// Test Hessian
	CHECK(Approx(hessian[0][0](0,0)) == -sin(x[0]));
	CHECK(Approx(hessian[0][0](1,1)) == -cos(x[1]));
	CHECK(Approx(hessian[0][0](0,1)) == 1.4);
	CHECK(Approx(hessian[0][0](1,0)) == 1.4);
}

class MyFunctor2
{
public:
	template<typename R>
	R operator()(const R* const x, const R* const y) const
	{
		return sin(x[0]) + cos(y[0]) + R(1.4)*x[0]*y[0] + R(1.0);
	}
};

TEST_CASE("AutoDiffTerm/MyFunctor2", "")
{
	AutoDiffTerm<MyFunctor2, 1, 1> term;

	double x = 5.3;
	double y = 7.1;
	std::vector<double*> variables;
	variables.push_back(&x);
	variables.push_back(&y);

	std::vector<Eigen::VectorXd> gradient;
	gradient.push_back(Eigen::VectorXd(1));
	gradient.push_back(Eigen::VectorXd(1));

	std::vector< std::vector<Eigen::MatrixXd> > hessian(2);
	hessian[0].resize(2);
	hessian[1].resize(2);
	hessian[0][0].resize(1,1);
	hessian[0][1].resize(1,1);
	hessian[1][0].resize(1,1);
	hessian[1][1].resize(1,1);

	double value  = term.evaluate(&variables[0], &gradient);
	double value2 = term.evaluate(&variables[0]);

	// The two values must agree.
	CHECK(Approx(value) == value2);

	// Test function value
	CHECK(Approx(value) == sin(x) + cos(y) + 1.4*x*y + 1.0);

	// Test gradient
	CHECK(Approx(gradient[0](0)) ==  cos(x) + 1.4*y);
	CHECK(Approx(gradient[1](0)) == -sin(y) + 1.4*x);

	for (auto& g : gradient) {
		g.setZero();
	}
	double value3 = term.evaluate(&variables[0], &gradient, &hessian);
	CHECK(Approx(value3) == value2);

	// Test gradient
	CHECK(Approx(gradient[0](0)) == cos(x) + 1.4*y);
	CHECK(Approx(gradient[1](0)) == -sin(y) + 1.4*x);

	// Test Hessian
	CHECK(Approx(hessian[0][0](0,0)) == -sin(x));
	CHECK(Approx(hessian[1][1](0,0)) == -cos(y));
	CHECK(Approx(hessian[1][0](0,0)) == 1.4);
	CHECK(Approx(hessian[0][1](0,0)) == 1.4);
}

class MyFunctor3
{
public:
	template<typename R>
	R operator()(const R* const x,
	             const R* const y,
	             const R* const z) const
	{
		return 2.0 * x[0]
		     + 2.0 * y[0] + 3.0 * y[1]
		     + 2.0 * z[0]*z[0] + 3.0 * z[1]*z[1] + 4.0 * z[2]*z[2];
	}
};

TEST_CASE("AutoDiffTerm/MyFunctor3")
{
	AutoDiffTerm<MyFunctor3, 1, 2, 3> term;

	double x[1] = {5.3};
	double y[2] = {7.1, 5.1};
	double z[3] = {9.5, 1.1, 5.2};
	std::vector<double*> variables;
	variables.push_back(x);
	variables.push_back(y);
	variables.push_back(z);

	std::vector<Eigen::VectorXd> gradient;
	gradient.push_back(Eigen::VectorXd(1));
	gradient.push_back(Eigen::VectorXd(2));
	gradient.push_back(Eigen::VectorXd(3));

	std::vector< std::vector<Eigen::MatrixXd> > hessian(3);
	hessian[0].resize(3);
	hessian[1].resize(3);
	hessian[2].resize(3);
	hessian[0][0].resize(1,1);
	hessian[0][1].resize(1,2);
	hessian[0][2].resize(1,3);
	hessian[1][0].resize(2,1);
	hessian[2][0].resize(3,1);
	hessian[1][1].resize(2,2);
	hessian[1][2].resize(2,3);
	hessian[2][1].resize(3,2);
	hessian[2][2].resize(3,3);

	double value  = term.evaluate(&variables[0], &gradient);
	double value2 = term.evaluate(&variables[0]);

	// The two values must agree.
	CHECK(Approx(value) == value2);

	// Test function value
	CHECK(Approx(value) == 
	         ( 2.0 * x[0]
		     + 2.0 * y[0] + 3.0 * y[1]
		     + 2.0 * z[0]*z[0] + 3.0 * z[1]*z[1] + 4.0 * z[2]*z[2]));

	// Test gradient
	CHECK(Approx(gradient[0](0)) ==  2.0);
	CHECK(Approx(gradient[1](0)) ==  2.0); 
	CHECK(Approx(gradient[1](1)) ==  3.0); 
	CHECK(Approx(gradient[2](0)) ==  2.0 * 2.0 * z[0]); 
	CHECK(Approx(gradient[2](1)) ==  2.0 * 3.0 * z[1]);
	CHECK(Approx(gradient[2](2)) ==  2.0 * 4.0 * z[2]);

	for (auto& g : gradient) {
		g.setZero();
	}
	double value3 = term.evaluate(&variables[0], &gradient, &hessian);
	CHECK(Approx(value3) == value2);

	// Test gradient
	CHECK(Approx(gradient[0](0)) == 2.0);
	CHECK(Approx(gradient[1](0)) == 2.0);
	CHECK(Approx(gradient[1](1)) == 3.0);
	CHECK(Approx(gradient[2](0)) == 2.0 * 2.0 * z[0]);
	CHECK(Approx(gradient[2](1)) == 2.0 * 3.0 * z[1]);
	CHECK(Approx(gradient[2](2)) == 2.0 * 4.0 * z[2]);

	// Test Hessian
	CHECK(Approx(hessian[0][0](0,0)) == 0.0);

	CHECK(Approx(hessian[1][1](0,0)) == 0.0);
	CHECK(Approx(hessian[1][1](0,1)) == 0.0);
	CHECK(Approx(hessian[1][1](1,0)) == 0.0);
	CHECK(Approx(hessian[1][1](1,1)) == 0.0);

	CHECK(Approx(hessian[2][2](0,0)) == 2.0 * 2.0);
	CHECK(Approx(hessian[2][2](1,1)) == 2.0 * 3.0);
	CHECK(Approx(hessian[2][2](2,2)) == 2.0 * 4.0);
}

class MyFunctor4
{
public:
	template<typename R>
	R operator()(const R* const x,
	             const R* const y,
	             const R* const z,
	             const R* const w) const
	{
		return 2.0 * x[0]*x[0]
		     + 2.0 * y[0]*y[0] + 3.0 * y[1]*y[1]
		     + 2.0 * z[0]*z[0] + 3.0 * z[1]*z[1] + 4.0 * z[2]*z[2]
			 + 2.0 * w[0]*z[0] + 3.0 * w[1]*z[1] + 4.0 * w[2]*z[2] + 5.0 * w[3]*w[3];
	}
};

TEST_CASE("AutoDiffTerm/MyFunctor4")
{
	AutoDiffTerm<MyFunctor4, 1, 2, 3, 4> term;

	double x[1] = {5.3};
	double y[2] = {7.1, 5.1};
	double z[3] = {9.5, 1.1, 5.2};
	double w[4] = {2.1, 7.87, 2.0, -1.9};
	std::vector<double*> variables;
	variables.push_back(x);
	variables.push_back(y);
	variables.push_back(z);
	variables.push_back(w);

	std::vector<Eigen::VectorXd> gradient;
	gradient.push_back(Eigen::VectorXd(1));
	gradient.push_back(Eigen::VectorXd(2));
	gradient.push_back(Eigen::VectorXd(3));
	gradient.push_back(Eigen::VectorXd(4));

	std::vector< std::vector<Eigen::MatrixXd> > hessian(4);
	hessian[0].resize(4);
	hessian[1].resize(4);
	hessian[2].resize(4);
	hessian[3].resize(4);

	hessian[0][0].resize(1,1);
	hessian[0][1].resize(1,2);
	hessian[0][2].resize(1,3);
	hessian[0][3].resize(1,4);
	hessian[1][0].resize(2,1);
	hessian[2][0].resize(3,1);
	hessian[3][0].resize(4,1);

	hessian[1][1].resize(2,2);
	hessian[1][2].resize(2,3);
	hessian[1][3].resize(2,4);
	hessian[2][1].resize(3,2);
	hessian[3][1].resize(4,2);

	hessian[2][2].resize(3,3);
	hessian[2][3].resize(3,4);
	hessian[3][2].resize(4,3);

	hessian[3][3].resize(4,4);

	double value  = term.evaluate(&variables[0], &gradient);
	double value2 = term.evaluate(&variables[0]);

	// The two values must agree.
	CHECK(Approx(value) == value2);

	// Test gradient
	CHECK(Approx(gradient[0](0)) ==  2.0 * 2.0 * x[0]);

	CHECK(Approx(gradient[1](0)) ==  2.0 * 2.0 * y[0]);
	CHECK(Approx(gradient[1](1)) ==  2.0 * 3.0 * y[1]);

	CHECK(Approx(gradient[2](0)) ==  2.0 * 2.0 * z[0] + 2.0 * w[0]); 
	CHECK(Approx(gradient[2](1)) ==  2.0 * 3.0 * z[1] + 3.0 * w[1]);
	CHECK(Approx(gradient[2](2)) ==  2.0 * 4.0 * z[2] + 4.0 * w[2]);

	CHECK(Approx(gradient[3](0)) ==  2.0 * z[0]); 
	CHECK(Approx(gradient[3](1)) ==  3.0 * z[1]);
	CHECK(Approx(gradient[3](2)) ==  4.0 * z[2]);
	CHECK(Approx(gradient[3](3)) ==  2.0 * 5.0 * w[3]);

	for (auto& g : gradient) {
		g.setZero();
	}
	double value3 = term.evaluate(&variables[0], &gradient, &hessian);
	CHECK(Approx(value3) == value2);

	// Test gradient
	CHECK(Approx(gradient[0](0)) == 2.0 * 2.0 * x[0]);

	CHECK(Approx(gradient[1](0)) == 2.0 * 2.0 * y[0]);
	CHECK(Approx(gradient[1](1)) == 2.0 * 3.0 * y[1]);

	CHECK(Approx(gradient[2](0)) == 2.0 * 2.0 * z[0] + 2.0 * w[0]);
	CHECK(Approx(gradient[2](1)) == 2.0 * 3.0 * z[1] + 3.0 * w[1]);
	CHECK(Approx(gradient[2](2)) == 2.0 * 4.0 * z[2] + 4.0 * w[2]);

	CHECK(Approx(gradient[3](0)) == 2.0 * z[0]);
	CHECK(Approx(gradient[3](1)) == 3.0 * z[1]);
	CHECK(Approx(gradient[3](2)) == 4.0 * z[2]);
	CHECK(Approx(gradient[3](3)) == 2.0 * 5.0 * w[3]);

	// Test Hessian
	CHECK(Approx(hessian[0][0](0,0)) == 2.0 * 2.0);

	CHECK(Approx(hessian[1][1](0,0)) == 2.0 * 2.0);
	CHECK(Approx(hessian[1][1](0,1)) == 0.0);
	CHECK(Approx(hessian[1][1](1,0)) == 0.0);
	CHECK(Approx(hessian[1][1](1,1)) == 2.0 * 3.0);

	CHECK(Approx(hessian[3][3](0,0)) == 0.0);
	CHECK(Approx(hessian[3][3](1,1)) == 0.0);
	CHECK(Approx(hessian[3][3](2,2)) == 0.0);
	CHECK(Approx(hessian[3][3](3,3)) == 2.0 * 5.0);
	CHECK(Approx(hessian[3][3](0,1)) == 0.0);
	CHECK(Approx(hessian[3][3](0,2)) == 0.0);
	CHECK(Approx(hessian[3][3](0,3)) == 0.0);
	CHECK(Approx(hessian[3][3](1,0)) == 0.0);
	CHECK(Approx(hessian[3][3](1,2)) == 0.0);
	CHECK(Approx(hessian[3][3](1,3)) == 0.0);
	CHECK(Approx(hessian[3][3](3,2)) == 0.0);

	CHECK(Approx(hessian[2][3](0,0)) == 2.0);
	CHECK(Approx(hessian[2][3](1,1)) == 3.0);
	CHECK(Approx(hessian[2][3](2,2)) == 4.0);
	CHECK(Approx(hessian[2][3](1,0)) == 0.0);
	CHECK(Approx(hessian[2][3](1,2)) == 0.0);

	CHECK(Approx(hessian[3][2](0,0)) == 2.0);
	CHECK(Approx(hessian[3][2](1,1)) == 3.0);
	CHECK(Approx(hessian[3][2](2,2)) == 4.0);
	CHECK(Approx(hessian[3][2](1,0)) == 0.0);
	CHECK(Approx(hessian[3][2](1,2)) == 0.0);
}

struct WriteFunctor1
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return 0.;
	}

	template<typename R>
	R operator()(const R* const x, const R* const y) const
	{
		return 0.;
	}

	void write(std::ostream& out) const
	{
		out << "Petter";
	}
};


TEST_CASE("AutoDiffTerm/write_test1", "")
{
	std::string file;

	std::stringstream fout;
	Term* term1 = new AutoDiffTerm<WriteFunctor1, 1>;
	fout << *term1;
	delete term1;
	file = fout.str();
		
	std::stringstream fin{ file };
	std::string petter;
	fin >> petter;
	CHECK(petter == "Petter");
}

TEST_CASE("AutoDiffTerm/write_test1_1", "")
{
	std::string file;

	std::stringstream fout;
	Term* term1_1 = new AutoDiffTerm<WriteFunctor1, 1, 1>;
	fout << *term1_1;
	delete term1_1;
	file = fout.str();
		
	std::stringstream fin{file};
	std::string petter;
	fin >> petter;
	CHECK(petter == "Petter");
}

struct ReadFunctor
{
	int* n;

	ReadFunctor(int* n_in) : n(n_in) { }

	template<typename R>
	R operator()(const R* const x) const
	{
		return 0.;
	}

	template<typename R>
	R operator()(const R* const x, const R* const y) const
	{
		return 0.;
	}

	void read(std::istream& in)
	{
		in >> *n;
	}
};

TEST_CASE("AutoDiffTerm/read_test", "")
{
	std::string file;

	std::stringstream fout;
	fout << 42;
	file = fout.str();

	int n = 0;
	Term* term1 = new AutoDiffTerm<ReadFunctor, 1>(&n);
	std::stringstream fin{file};
	fin >> *term1;
	CHECK(n == 42);
	delete term1;
	
	int m = 0;
	Term* term1_1 = new AutoDiffTerm<ReadFunctor, 1, 1>(&m);
	std::stringstream fin2{file};
	fin2 >> *term1_1;
	CHECK(m == 42);
	delete term1_1;
}


template<int... D>
class EmptySizedTerm
	: public SizedTerm<D...>
{
public:
	virtual double evaluate(double * const * const variables) const override
	{
		return 0;
	}
	virtual double evaluate(double * const * const variables,
	                        std::vector<Eigen::VectorXd>* gradient) const override
	{
		return 0;
	}
	virtual double evaluate(double * const * const variables,
	                        std::vector<Eigen::VectorXd>* gradient,
	                        std::vector< std::vector<Eigen::MatrixXd> >* hessian)
	                        const override
	{
		return 0;
	}
};

TEST_CASE("SizedTerm::number_of_variables")
{
	CHECK((EmptySizedTerm<1>{}.number_of_variables()) == 1);
	CHECK((EmptySizedTerm<5>{}.number_of_variables()) == 1);
	CHECK((EmptySizedTerm<1, 1>{}.number_of_variables()) == 2);
	CHECK((EmptySizedTerm<5, 5>{}.number_of_variables()) == 2);
	CHECK((EmptySizedTerm<1, 2, 3, 4, 5, 6, 7>{}.number_of_variables()) == 7);
}

TEST_CASE("AutoDiffTerm::number_of_variables")
{
	CHECK((AutoDiffTerm<Func, 1>{}.number_of_variables()) == 1);
	CHECK((AutoDiffTerm<MyFunctor4, 1, 1, 1, 1>{}.number_of_variables()) == 4);
}


class MyFunctor_1_1_1_2_2_4_4
{
public:
	template<typename R>
	R operator()(
		const R* const x1,
		const R* const x2,
		const R* const x3,
		const R* const x4,
		const R* const x5,
		const R* const x6,
		const R* const x7) const
	{
		return
			3 * x1[0] +
			5 * x2[0] +
			9 * x3[0] +
			3 * x4[0] + 4 * x4[1] +
			1 * x5[0] + 6 * x5[1] +
			4 * x6[0] + 3 * x6[1] + 2 * x6[2] + 2 * x6[3] +
			2 * x7[0] + 1 * x7[1] + 8 * x7[2] + 9 * x7[3];
	}
};

// Tests the variadic template version of AutoDiffTerm.
TEST_CASE("AutoDiffTerm/MyFunctor_1_1_1_2_2_4_4", "")
{
	AutoDiffTerm<MyFunctor_1_1_1_2_2_4_4, 1, 1, 1, 2, 2, 4, 4> term;

	double x1[1] = {5.3};
	double x2[1] = {1.7};
	double x3[1] = {0.8};
	double x4[2] = {2.9, 3.9};
	double x5[2] = {1.0, 2.0};
	double x6[4] = {1.3, 7.2, 4.5, 8.8};
	double x7[4] = {9.8, 2.3, 5.6, 1.1};
	std::vector<double*> variables =
		{x1, x2, x3, x4, x5, x6, x7};

	double value = term.evaluate(&variables[0]);
	MyFunctor_1_1_1_2_2_4_4 f;
	CHECK(value == f(x1, x2, x3, x4, x5, x6, x7));

	std::vector<Eigen::VectorXd> gradient(7);
	gradient[0].resize(1);
	gradient[1].resize(1);
	gradient[2].resize(1);
	gradient[3].resize(2);
	gradient[4].resize(2);
	gradient[5].resize(4);
	gradient[6].resize(4);
	auto value2 = term.evaluate(variables.data(), &gradient);
	CHECK(value2 == f(x1, x2, x3, x4, x5, x6, x7));
	CHECK(gradient[0][0] == 3);
	CHECK(gradient[1][0] == 5);
	CHECK(gradient[2][0] == 9);

	CHECK(gradient[3][0] == 3);
	CHECK(gradient[3][1] == 4);

	CHECK(gradient[4][0] == 1);
	CHECK(gradient[4][1] == 6);

	CHECK(gradient[5][0] == 4);
	CHECK(gradient[5][1] == 3);
	CHECK(gradient[5][2] == 2);
	CHECK(gradient[5][3] == 2);

	CHECK(gradient[6][0] == 2);
	CHECK(gradient[6][1] == 1);
	CHECK(gradient[6][2] == 8);
	CHECK(gradient[6][3] == 9);

	std::vector< std::vector<Eigen::MatrixXd>> empty_hessians;
	CHECK_THROWS(term.evaluate(variables.data(), &gradient, &empty_hessians));
}

struct DetectCopyFunctor
{
	static int num_constructions;
	static int num_copies;
	static int num_moves;
	
	double data = 0;

	DetectCopyFunctor(double data_)
		: data{data_}
	{
		num_constructions++;
	}

	DetectCopyFunctor(const DetectCopyFunctor& rhs)
	{
		num_copies++;
		this->data = rhs.data;
	}

	DetectCopyFunctor(DetectCopyFunctor&& rhs)
	{
		num_moves++;
		this->data = rhs.data;
	}

	template<typename R>
	R operator()(const R* x) const
	{
		return R{data};
	}
};

int DetectCopyFunctor::num_constructions = 0;
int DetectCopyFunctor::num_copies = 0;
int DetectCopyFunctor::num_moves = 0;

// This test makes sure that the std::forward machinery inside
// make_differentiable and AutoDiffTerm is working correctly.
TEST_CASE("AutoDiffTerm/CopiesMade")
{
	REQUIRE(DetectCopyFunctor::num_constructions == 0);
	REQUIRE(DetectCopyFunctor::num_copies == 0);
	REQUIRE(DetectCopyFunctor::num_moves == 0);

	DetectCopyFunctor functor{123};	
	CHECK(DetectCopyFunctor::num_constructions == 1);
	CHECK(DetectCopyFunctor::num_copies == 0);
	CHECK(DetectCopyFunctor::num_moves == 0);

	AutoDiffTerm<DetectCopyFunctor, 1> term1(123456);
	CHECK(DetectCopyFunctor::num_constructions == 2);
	CHECK(DetectCopyFunctor::num_copies == 0);
	CHECK(DetectCopyFunctor::num_moves == 0);

	AutoDiffTerm<DetectCopyFunctor, 1> term2(functor);
	CHECK(DetectCopyFunctor::num_constructions == 2);
	CHECK(DetectCopyFunctor::num_copies == 1);
	CHECK(DetectCopyFunctor::num_moves == 0);

	auto term3 = make_differentiable<1>(functor);
	CHECK(DetectCopyFunctor::num_constructions == 2);
	CHECK(DetectCopyFunctor::num_copies == 2);
	CHECK(DetectCopyFunctor::num_moves == 0);

	// Make sure that make_differentiable actually has
	// made a copy of the functor. This can be achieved
	// with std::remove_reference.
	double x[1] = {0};
	std::vector<double*> vars = {x};
	CHECK(term3->evaluate(vars.data()) == 123);
	functor.data = 0;
	CHECK(term3->evaluate(vars.data()) == 123);

	AutoDiffTerm<DetectCopyFunctor, 1> term4{DetectCopyFunctor{123}};
	CHECK(DetectCopyFunctor::num_constructions == 3);
	CHECK(DetectCopyFunctor::num_copies == 2);
	CHECK(DetectCopyFunctor::num_moves == 1);

	auto term5 = make_differentiable<1>(DetectCopyFunctor{456});
	CHECK(DetectCopyFunctor::num_constructions == 4);
	CHECK(DetectCopyFunctor::num_copies == 2);
	CHECK(DetectCopyFunctor::num_moves == 2);
}

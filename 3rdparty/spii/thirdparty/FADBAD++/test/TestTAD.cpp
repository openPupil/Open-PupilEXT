#include "tadiff.h"
#include "extra/ndf.h"
#include "extra/ndfad.h"

#include "TestTAD.h"

#include <iostream>

using namespace std;
using namespace fadbad;

template <typename F, int ORDER>
class DIntegral
{
	F m_func;                       // Function to integrate
public:
	DIntegral():m_func(){}
	DIntegral(const F& f):m_func(f){}
	template <typename aType>
	aType operator ()(const aType a, const aType b)
	{
		T< aType > x,fx;
		aType h2((b-a)/2),res;
		x=(a+b)/2;              // Point of evaluation
		x[1]=1;                 // dx/dx=1
		fx=m_func(x);           // Record function
		fx.eval(ORDER);         // Compute up to ORDER coefficients
		res=0;
		// Evaluate the integral from a to b of the Taylor-polynomial.
		for(int i=ORDER;i>=0;i--)
			res+=fx[i]*(pow(h2,i+1)-pow(-h2,i+1))/(i+1);
		return res;
	}
};

template <typename F, unsigned int ORDER=5, unsigned int N=10>
class Integral
{
	DIntegral<F,ORDER> m_dInt;
public:
	Integral():m_dInt(){}
	Integral(const F& f):m_dInt(f){}
	template <typename aType>
	aType operator ()(const aType a, const aType b)
	{
		unsigned int i,j;
		aType res=0;
		for(i=0;i<N;i++)
		{
			j=i+1;
			res+=m_dInt((a*(N-i)+b*i)/N,(a*(N-j)+b*j)/N);
		}
		return res;
	}

};

template <typename F, typename U>
class TaylorODE
{
	T<U> in[F::n];
	T<U> out[F::n];
public:
	TaylorODE(){ F(in,out); }
	void reset()
	{
		for(unsigned int j=0;j<F::n;++j) out[j].reset();
	}
	U& operator[](const unsigned int j){ return in[j][0]; }
	void init() { F::init(in); }
	void eval(unsigned int order)
	{
		for(unsigned int i=0;i<order;++i)
		{
			for(unsigned int j=0;j<F::n;++j)
			{
				out[j].eval(i);
				in[j][i+1]=out[j][i]/(i+1);
			}
		}
	}	
	std::ostream& print(std::ostream& ost) const
	{
		for(unsigned int j=0;j<F::n;++j)
		{
			ost<<j<<" :";
			for(unsigned int i=0;i<in[j].length();++i) ost<<" "<<in[j][i];
			ost<<std::endl;
		}
	}
	bool operator!=(const TaylorODE<F,U>& other)
	{
		for(unsigned int j=0;j<F::n;++j)
		{
			if (in[j].length()!=other.in[j].length()) return true;
			if (out[j].length()!=other.out[j].length()) return true;
		}
		for(unsigned int j=0;j<F::n;++j)
		{
			for(unsigned int i=0;i<in[j].length();++i)
			{
				if (in[j][i]!=other.in[j][i]) return true;
			}
			for(unsigned int i=0;i<out[j].length();++i)
			{
				if (out[j][i]!=other.out[j][i]) return true;
			}
		}
		return false;
	}
};

class F1
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		aType exx(exp(x*x));
		return 2.0*x*exx*sin(exx);
	}
};

class F2
{
	double m_a;
public:
	F2(const double& a):m_a(a){}
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return 1/sqrt(m_a*m_a-x*x);
	}
};

class F3
{
	double m_a;
public:
	F3(const double& a):m_a(a){}
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return sqrt(m_a*m_a-x*x);
	}
};

class F4
{
	double m_a;
public:
	F4(const double& a):m_a(a){}
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return 1/(1-2*m_a*cos(x)+m_a*m_a);
	}
};

class F5
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return log(1-x)/x;
	}
};

class F6
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return log(exp(x));
	}
};

class F7
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return sin(asin(x));
	}
};

class F8
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return cos(acos(x));
	}
};

class F9
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return atan(tan(x));
	}
};

class F10
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return sqrt(sqr(x));
	}
};

class F11
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return icnd(cnd(x));
	}
};

class F12
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return exp(log(pow(x,x))/x);
	}
};

class F13
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return exp(log(pow(x,1.234))/1.234);
	}
};

class F14
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return log(pow(1.234,x))/log(1.234);
	}
};

class F15
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return (x+1.465)-1.465;
	}
};

class F16
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return (1.465+x)-1.465;
	}
};

class F17
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return (x-1.465)+1.465;
	}
};

class F18
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return -(1.465-x)+1.465;
	}
};

class F19
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return (x*1.465)/1.465;
	}
};

class F20
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return (1.465*x)/1.465;
	}
};

class F21
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return (x/1.465)*1.465;
	}
};

class F22
{
public:
	template <typename aType>
	aType operator ()(const aType& x)
	{
		return 1.0/((1.465/x)/1.465);
	}
};

struct F23
{
	static const int n=2;
	template <typename U>
	static void init(U* in)
	{
		in[0][0]=sin(0.0);
		in[1][0]=cos(0.0);
		
	}
	template <typename U>
	F23(const U* in, U* out)
	{
		out[0]=+in[1];
		out[1]=-in[0];
	}
};

struct F24 // Lorenz
{
	static const int n=3;
	template <typename U>
	static void init(U* in)
	{
		in[0][0]=1;
		in[1][0]=1;
		in[2][0]=1;
		
	}
	template <typename U>
	F24(const U* y, U* yp)
	{
		double sigma(10.0),rho(28.0);
		double beta(8.0/3.0);

		yp[0] = sigma*(y[1]-y[0]);
		yp[1] = y[0]*(rho-y[2])-y[1];
		yp[2] = y[0]*y[1]-beta*y[2];
	}
};



void TestTAD::run(IReportLog& rlog)
{
	double numerical, analytical;

	numerical=Integral<F1,6,160>()(0.0,2.0);
	analytical=cos(1.0)-cos(exp(4.0));
//	cout << "Numerical:  " << numerical << endl;
//	cout << "Analytical: " << analytical << endl;
	if (fabs(numerical-analytical)>0.01)
		rlog.failed() << "Integral of F1 failed" << endl;
	else
		rlog.succeeded() << "Integral of F1 succeeded" << endl;


	double a=5;
	F2 f2(a);
	numerical=Integral<F2,10,500>(f2)(0.0,a);
	analytical=3.141592653589793/2;
//	cout << "Numerical:  " << numerical << endl;
//	cout << "Analytical: " << analytical << endl;
	if (fabs(numerical-analytical)>0.01)
		rlog.failed() << "Integral of F2 failed" << endl;
	else
		rlog.succeeded() << "Integral of F2 succeeded" << endl;

	F3 f3(a);
	numerical=Integral<F3,6,160>(f3)(0.0,a);
	analytical=3.141592653589793*a*a/4;
//	cout << "Numerical:  " << numerical << endl;
//	cout << "Analytical: " << analytical << endl;
	if (fabs(numerical-analytical)>0.01)
		rlog.failed() << "Integral of F3 failed" << endl;
	else
		rlog.succeeded() << "Integral of F3 succeeded" << endl;

	a=0.4;
	F4 f4(a);
	numerical=Integral<F4,6,160>(f4)(0.0,2*3.141592653589793);
	analytical=2*3.141592653589793/(1-a*a);
//	cout << "Numerical:  " << numerical << endl;
//	cout << "Analytical: " << analytical << endl;
	if (fabs(numerical-analytical)>0.01)
		rlog.failed() << "Integral of F4 failed" << endl;
	else
		rlog.succeeded() << "Integral of F4 succeeded" << endl;

	numerical=Integral<F5,6,160>()(0.0,1.0);
	analytical=-pow(3.141592653589793,2)/6;
//	cout << "Numerical:  " << numerical << endl;
//	cout << "Analytical: " << analytical << endl;
	if (fabs(numerical-analytical)>0.01)
		rlog.failed() << "Integral of F5 failed" << endl;
	else
		rlog.succeeded() << "Integral of F5 succeeded" << endl;

	T<double> x,y;
	x[0]=0.123;
	x[1]=1;
	y=F6()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-10)
		{
			rlog.failed() << "F6 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F6 succeeded" << endl;
	}

	y=F7()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-10)
		{
			rlog.failed() << "F7 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F7 succeeded" << endl;
	}

	y=F8()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-10)
		{
			rlog.failed() << "F8 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F8 succeeded" << endl;
	}


	y=F9()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-10)
		{
			rlog.failed() << "F9 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F9 succeeded" << endl;
	}

	y=F10()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-10)
		{
			rlog.failed() << "F10 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F10 succeeded" << endl;
	}

	y=F11()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-10)
		{
			rlog.failed() << "F11 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F11 succeeded" << endl;
	}

	y=F12()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-7)
		{
			rlog.failed() << "F12 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F12 succeeded" << endl;
	}

	y=F13()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-7)
		{
			rlog.failed() << "F13 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F13 succeeded" << endl;
	}

	y=F14()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-7)
		{
			rlog.failed() << "F14 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F14 succeeded" << endl;
	}

	y=F15()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-7)
		{
			rlog.failed() << "F15 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F15 succeeded" << endl;
	}

	y=F16()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-7)
		{
			rlog.failed() << "F16 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F16 succeeded" << endl;
	}

	y=F17()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-7)
		{
			rlog.failed() << "F17 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F17 succeeded" << endl;
	}

	y=F18()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-7)
		{
			rlog.failed() << "F18 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F18 succeeded" << endl;
	}

	y=F19()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-7)
		{
			rlog.failed() << "F19 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F19 succeeded" << endl;
	}

	y=F20()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-7)
		{
			rlog.failed() << "F20 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F20 succeeded" << endl;
	}

	y=F21()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-7)
		{
			rlog.failed() << "F21 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F21 succeeded" << endl;
	}

	y=F22()(x);
	y.eval(10);
	for(int i=0;i<=10;++i)
	{
		if (fabs(y[i]-x[i])>1e-7)
		{
			rlog.failed() << "F22 "<<i<<"'th order failed" << endl;
			break;
		}
		if (i==10) rlog.succeeded() << "F22 succeeded" << endl;
	}



	TaylorODE<F23,double> TF23a;
	TF23a.reset();
	TF23a.init();
//	TF23a.print(std::cout);
	TF23a.eval(10);
//	TF23a.print(std::cout);

	TaylorODE<F23,double> TF23b;
	TF23b.init();
	TF23b.reset();
//	TF23b.print(std::cout);
	TF23b.eval(10);
//	TF23b.print(std::cout);

	if (TF23a!=TF23b)
	{
		rlog.failed() << "F23 failed" << endl;
	}
	else
	{
		rlog.succeeded() << "F23 succeeded" << endl;
	}
	
	TaylorODE<F24,double> TF24a;
	TaylorODE<F24,double> TF24b;
	TF24a.reset();
	TF24a.init();
	TF24a.eval(10);
//	TF24a.print(std::cout);

	TF24b.reset();
	TF24b.init();
	TF24b.eval(10);
//	TF24b.print(std::cout);

	TF24b.init();
	TF24b.reset();
	TF24b.eval(10);
//	TF24b.print(std::cout);

	if (TF24a!=TF24b)
	{
		rlog.failed() << "F24 failed" << endl;
	}
	else
	{
		rlog.succeeded() << "F24 succeeded" << endl;
	}



}

















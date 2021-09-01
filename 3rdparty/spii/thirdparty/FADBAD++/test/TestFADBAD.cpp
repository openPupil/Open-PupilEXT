#include "badiff.h" // Backwards automatic differentiation
#include "fadiff.h" // Forwards automatic differentiation

#include "extra/ndf.h"
#include "extra/ndfad.h"

#include <vector>
#include "TestFADBAD.h"

using namespace fadbad;

// Functions to differentiate:

struct Add
{
	static const char* Name(){ return "T operator +(const T&, const T&)"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0]+v[1];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		in[1]=-1.5;
		return in;
	}
};
struct Add1
{
	static const char* Name(){ return "T operator +(const T&, const T&) same object"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]=out[0]+out[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct CAdd
{
	static const char* Name(){ return "T& T::operator +=(const T&)"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]+=v[1];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		in[1]=-1.5;
		return in;
	}
};
struct CAdd1
{
	static const char* Name(){ return "T& T::operator +=(const double)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]+=117.0;
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct CAdd2
{
	static const char* Name(){ return "T& T::operator +=(const T&) same instance"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]+=out[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Add2
{
	static const char* Name(){ return "T operator +(const T&, const double)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0]+117.0;
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Add3
{
	static const char* Name(){ return "T operator +(const double, const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=118.0+v[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Sub
{
	static const char* Name(){ return "T operator -(const T&, const T&)"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0]-v[1];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		in[1]=-1.5;
		return in;
	}
};
struct Sub1
{
	static const char* Name(){ return "T operator -(const T&, const T&) same object"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]=out[0]-out[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct CSub
{
	static const char* Name(){ return "T& T::operator -=(const T&)"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]-=v[1];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		in[1]=-1.5;
		return in;
	}
};
struct CSub1
{
	static const char* Name(){ return "T& T::operator -=(const double)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]-=117.0;
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct CSub2
{
	static const char* Name(){ return "T& T::operator -=(const T&) same instance"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]-=out[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Sub2
{
	static const char* Name(){ return "T operator -(const T&, const double)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0]-117.0;
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Sub3
{
	static const char* Name(){ return "T operator -(const double, const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=118.0-v[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Mul
{
	static const char* Name(){ return "T operator *(const T&, const T&)"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0]*v[1];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		in[1]=-1.5;
		return in;
	}
};
struct Mul1
{
	static const char* Name(){ return "T operator *(const T&, const T&) same object"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]=out[0]*out[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct CMul
{
	static const char* Name(){ return "T& T::operator *=(const T&)"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]*=v[1];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		in[1]=-1.5;
		return in;
	}
};
struct CMul1
{
	static const char* Name(){ return "T& T::operator *=(const double)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]*=117.0;
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct CMul2
{
	static const char* Name(){ return "T& T::operator *=(const T&) same instance"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]*=out[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Mul2
{
	static const char* Name(){ return "T operator *(const T&, const double)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0]*117.0;
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Mul3
{
	static const char* Name(){ return "T operator *(const double, const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=118.0*v[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Div
{
	static const char* Name(){ return "T operator /(const T&, const T&)"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0]/v[1];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		in[1]=-1.5;
		return in;
	}
};
struct Div1
{
	static const char* Name(){ return "T operator /(const T&, const T&) same object"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]=out[0]/out[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct CDiv
{
	static const char* Name(){ return "T& T::operator /=(const T&)"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]/=v[1];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		in[1]=-1.5;
		return in;
	}
};
struct CDiv1
{
	static const char* Name(){ return "T& T::operator /=(const double)"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]/=117.0;
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct CDiv2
{
	static const char* Name(){ return "T& T::operator /=(const T&) same instance"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0];
		out[0]/=out[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Div2
{
	static const char* Name(){ return "T operator /(const T&, const double)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=v[0]/117.0;
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Div3
{
	static const char* Name(){ return "T operator /(const double, const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=118.0/v[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Pow
{
	static const char* Name(){ return "T pow(const T&, int)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=pow(v[0],3.0);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Pow2
{
	static const char* Name(){ return "T pow(const T&, const T&)"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=pow(v[0],v[1]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		in[0]=3.3;
		return in;
	}
};
struct UAdd
{
	static const char* Name(){ return "T sqr(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=+v[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct USub
{
	static const char* Name(){ return "T sqr(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=-v[0];
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Sqr
{
	static const char* Name(){ return "T sqr(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=sqr(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Exp
{
	static const char* Name(){ return "T exp(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=exp(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Log
{
	static const char* Name(){ return "T log(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=log(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Sqrt
{
	static const char* Name(){ return "T sqrt(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=sqrt(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Sin
{
	static const char* Name(){ return "T sin(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=sin(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Cos
{
	static const char* Name(){ return "T cos(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=cos(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct Tan
{
	static const char* Name(){ return "T tan(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=tan(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		return in;
	}
};
struct ASin
{
	static const char* Name(){ return "T asin(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=asin(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=0.5;
		return in;
	}
};
struct ACos
{
	static const char* Name(){ return "T acos(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=acos(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=0.5;
		return in;
	}
};
struct ATan
{
	static const char* Name(){ return "T atan(const T&)"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=atan(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=0.5;
		return in;
	}
};
struct Nd
{
	static const char* Name(){ return "T nd(const T&) -- normal distribution"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=nd(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=0.5;
		return in;
	}
};
struct Cnd
{
	static const char* Name(){ return "T cnd(const T&) -- cumulative normal distribution"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=cnd(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=0.5;
		return in;
	}
};
struct Icnd
{
	static const char* Name(){ return "T cnd(const T&) -- invers cumulative normal distribution"; }
	static const int m=1;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=icnd(v[0]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=0.8;
		return in;
	}
};



struct csmap // The cos-sin map.
{
	static const char* Name(){ return "Cos-Sine map"; }
	static const int m=2;
	static const int n=2;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		out[0]=cos(v[0]+4*v[1]);
		out[1]=sin(4*v[0]+v[1]);
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		in[1]=-1.5;
		return in;
	}
};
struct func1
{
	static const char* Name(){ return "atan[(a sin x)/(1-a cos x)]"; }
	static const int m=2;
	static const int n=1;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		std::vector<T> out(n);
		T x(v[0]);
		T a(v[1]);
		out[0]=atan((a*sin(x))/(1-a*cos(x)));
		return out;
	}
	static std::vector<double> point()
	{
		std::vector< double > in(m);
		in[0]=1.5;
		in[1]=-.456;
		return in;
	}
};



// The classes Fdiff<C> and Bdiff are capable of differentiating a function
// f = [f0,f1,..,f(m-1)] : T^n->T^m by using either the forward or the backward
// modes of automatic differentiation. Where T is real, interval, etc.
// The argument when using the evaluating operation on Fdiff is an
// n-dimensional std::vector x in which the function should be evaluated
// and differentiated. The return value is a std::vector that should
// be interpreted as follows:
//		Vout[0..m-1] is the function value.
//		Vout[m..m+n-1] is the value of [df1/dx1, ..., df1/dxn]
//		Vout[m+i*n..m+(i+1)*n-1] is the value of [dfi/dx1, ..., dfi/dxn].

template <typename C>
struct Fdiff // Use heap-based forward differentiation
{
	static const int m=C::m;
	static const int n=C::n+C::m*C::n;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		USER_ASSERT(v.size()==C::m,"UNEXPECTED SIZE")
		int i,j;
		std::vector< F<T> > out(C::n);
		std::vector< F<T> > in(C::m);
		for(i=0;i<C::m;++i)
		{
			in[i]=v[i];
			in[i].diff(i,C::m);
		}
		out=C()(in);
		std::vector<T> retval(n);
		for(j=0;j<C::n;++j)
		{
			retval[j]=out[j].x();
			for(i=0;i<C::m;++i)
			{
				retval[C::n+j*C::m+i]=out[j].d(i);
			}
		}
		return retval;
	}
};

template <typename C>
struct FFdiff // Use stack-based forward differentiation
{
	static const int m=C::m;
	static const int n=C::n+C::m*C::n;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		USER_ASSERT(v.size()==C::m,"UNEXPECTED SIZE")
		int i,j;
		std::vector< F<T,C::m> > out(C::n);
		std::vector< F<T,C::m> > in(C::m);
		for(i=0;i<C::m;++i)
		{
			in[i]=v[i];
			in[i].diff(i);
		}
		out=C()(in);
		std::vector<T> retval(n);
		for(j=0;j<C::n;++j)
		{
			retval[j]=out[j].x();
			for(i=0;i<C::m;++i)
			{
				retval[C::n+j*C::m+i]=out[j].d(i);
			}
		}
		return retval;
	}
};

template <typename C>
struct Bdiff
{
	static const int m=C::m;
	static const int n=C::n+C::m*C::n;
	template <typename T>
	std::vector<T> operator()(const std::vector<T>& v)
	{
		USER_ASSERT(v.size()==C::m,"UNEXPECTED SIZE")
		int i,j;
		std::vector< B<T> > out(C::n);
		std::vector< B<T> > in(C::m);
		for(i=0;i<C::m;++i)
		{
			in[i]=v[i];
		}
		out=C()(in);
		for(j=0;j<C::n;++j)
		{
			out[j].diff(j,C::n);
		}
		std::vector<T> retval(n);
		for(j=0;j<C::n;++j)
		{
			retval[j]=out[j].x();
			for(i=0;i<C::m;++i)
			{
				retval[C::n+j*C::m+i]=in[i].d(j);
			}
		}
		return retval;
	}
};

template <typename T>
std::ostream& operator << (std::ostream& os, const std::vector<T>& v)
{
	os << "[";
	for(unsigned int i=0;i<v.size();i++)
	{
		os << v[i];
		if (i<v.size()-1) os << ",";
	}
	os << "]";
	return os;
}

template <typename T>
struct check
{
	void runFB(IReportLog& log);
	void runFFB(IReportLog& log);
};

bool diff(const std::vector<double> v1, const std::vector<double> v2)
{
//	std::cout<<"COMPARE"<<std::endl;
//	std::cout<<v1<<std::endl;
//	std::cout<<v2<<std::endl;

	if (v1.size()!=v2.size()) return true;
	for(unsigned int i=0;i<v1.size();++i)
		if (fabs(v1[i]-v2[i])>1e-6)
		{

			std::cout<<v1<<std::endl;
			std::cout<<v2<<std::endl;
			return true;
		}

	return false;
}

template <typename T>
void check<T>::runFB(IReportLog& log)
{
	std::vector<double> v1;
	std::vector<double> v2;
	std::vector<double> v3;

	{
		Fdiff< T > Fd;
		v1=Fd(T::point());
	}

	{
		Bdiff< T > Bd;
		v2=Bd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 1. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 1. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff< Fdiff < T > > FFd;
		v1=FFd(T::point());
	}

	{
		Bdiff< Fdiff < T > > BFd;
		v2=BFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 2. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 2. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff< Bdiff < T > > FBd;
		v2=FBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 2. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 2. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff< Bdiff < T > > BBd;
		v2=BBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 2. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 2. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff < Fdiff< Fdiff < T > > > FFFd;
		v1=FFFd(T::point());
	}

	{
		Bdiff < Fdiff< Fdiff < T > > > BFFd;
		v2=BFFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff < Bdiff< Fdiff < T > > > FBFd;
		v2=FBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff< Fdiff < T > > > BBFd;
		v2=BBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff < Fdiff< Bdiff < T > > > FFBd;
		v2=FFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Fdiff< Bdiff < T > > > BFBd;
		v2=BFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff < Bdiff< Bdiff < T > > > FBBd;
		v2=FBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff< Bdiff < T > > > BBBd;
		v2=BBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;
/* Uncomment this if you want to wait even longer for compilation to finish
	{
		Fdiff < Fdiff < Fdiff< Fdiff < T > > > > FFFFd;
		v1=FFFFd(T::point());
	}

	{
		Bdiff < Fdiff < Fdiff< Fdiff < T > > > > BFFFd;
		v2=BFFFd(T::point());
	}
	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff < Bdiff < Fdiff< Fdiff < T > > > > FBFFd;
		v2=FBFFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff < Fdiff< Fdiff < T > > > > BBFFd;
		v2=BBFFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff < Fdiff < Bdiff< Fdiff < T > > > > FFBFd;
		v2=FFBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Fdiff < Bdiff< Fdiff < T > > > > BFBFd;
		v2=BFBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff < Bdiff < Bdiff< Fdiff < T > > > > FBBFd;
		v2=FBBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff < Bdiff< Fdiff < T > > > > BBBFd;
		v2=BBBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff < Fdiff < Fdiff< Bdiff < T > > > > FFFBd;
		v2=FFFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Fdiff < Fdiff< Bdiff < T > > > > BFFBd;
		v2=BFFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff < Bdiff < Fdiff< Bdiff < T > > > > FBFBd;
		v2=FBFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff < Fdiff< Bdiff < T > > > > BBFBd;
		v2=BBFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff < Fdiff < Bdiff< Bdiff < T > > > > FFBBd;
		v2=FFBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Fdiff < Bdiff< Bdiff < T > > > > BFBBd;
		v2=BFBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Fdiff < Bdiff < Bdiff< Bdiff < T > > > > FBBBd;
		v2=FBBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff < Bdiff< Bdiff < T > > > > BBBBd;
		v2=BBBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;
*/
}

template <typename T>
void check<T>::runFFB(IReportLog& log)
{
	std::vector<double> v1;
	std::vector<double> v2;
	std::vector<double> v3;

	{
		FFdiff< T > Fd;
		v1=Fd(T::point());
	}

	{
		Bdiff< T > Bd;
		v2=Bd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 1. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 1. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff< FFdiff < T > > FFd;
		v1=FFd(T::point());
	}

	{
		Bdiff< FFdiff < T > > BFd;
		v2=BFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 2. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 2. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff< Bdiff < T > > FBd;
		v2=FBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 2. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 2. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff< Bdiff < T > > BBd;
		v2=BBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 2. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 2. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff < FFdiff< FFdiff < T > > > FFFd;
		v1=FFFd(T::point());
	}

	{
		Bdiff < FFdiff< FFdiff < T > > > BFFd;
		v2=BFFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff < Bdiff< FFdiff < T > > > FBFd;
		v2=FBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff< FFdiff < T > > > BBFd;
		v2=BBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff < FFdiff< Bdiff < T > > > FFBd;
		v2=FFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < FFdiff< Bdiff < T > > > BFBd;
		v2=BFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff < Bdiff< Bdiff < T > > > FBBd;
		v2=FBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff< Bdiff < T > > > BBBd;
		v2=BBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 3. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 3. order derivative succeeded for " << T::Name() << std::endl;
/* Uncomment this if you want to wait even longer for compilation to finish
	{
		FFdiff < FFdiff < FFdiff< FFdiff < T > > > > FFFFd;
		v1=FFFFd(T::point());
	}

	{
		Bdiff < FFdiff < FFdiff< FFdiff < T > > > > BFFFd;
		v2=BFFFd(T::point());
	}
	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff < Bdiff < FFdiff< FFdiff < T > > > > FBFFd;
		v2=FBFFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff < FFdiff< FFdiff < T > > > > BBFFd;
		v2=BBFFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff < FFdiff < Bdiff< FFdiff < T > > > > FFBFd;
		v2=FFBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < FFdiff < Bdiff< FFdiff < T > > > > BFBFd;
		v2=BFBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff < Bdiff < Bdiff< FFdiff < T > > > > FBBFd;
		v2=FBBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff < Bdiff< FFdiff < T > > > > BBBFd;
		v2=BBBFd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff < FFdiff < FFdiff< Bdiff < T > > > > FFFBd;
		v2=FFFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < FFdiff < FFdiff< Bdiff < T > > > > BFFBd;
		v2=BFFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff < Bdiff < FFdiff< Bdiff < T > > > > FBFBd;
		v2=FBFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff < FFdiff< Bdiff < T > > > > BBFBd;
		v2=BBFBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff < FFdiff < Bdiff< Bdiff < T > > > > FFBBd;
		v2=FFBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < FFdiff < Bdiff< Bdiff < T > > > > BFBBd;
		v2=BFBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		FFdiff < Bdiff < Bdiff< Bdiff < T > > > > FBBBd;
		v2=FBBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;

	{
		Bdiff < Bdiff < Bdiff< Bdiff < T > > > > BBBBd;
		v2=BBBBd(T::point());
	}

	if (diff(v1,v2)) log.failed() << "Testing 4. order derivative failed for " << T::Name() << std::endl;
	else log.succeeded() << "Testing 4. order derivative succeeded for " << T::Name() << std::endl;
*/
}



void TestFADBAD::run(IReportLog& rlog)
{
	{ check<Add> chk; chk.runFB(rlog); }
	{ check<Add1> chk; chk.runFB(rlog); }
	{ check<CAdd> chk; chk.runFB(rlog); }
	{ check<CAdd1> chk; chk.runFB(rlog); }
	{ check<CAdd2> chk; chk.runFB(rlog); }
	{ check<Add2> chk; chk.runFB(rlog); }
	{ check<Add3> chk; chk.runFB(rlog); }
	{ check<Sub> chk; chk.runFB(rlog); }
	{ check<Sub1> chk; chk.runFB(rlog); }
	{ check<CSub> chk; chk.runFB(rlog); }
	{ check<CSub1> chk; chk.runFB(rlog); }
	{ check<CSub2> chk; chk.runFB(rlog); }
	{ check<Sub2> chk; chk.runFB(rlog); }
	{ check<Sub3> chk; chk.runFB(rlog); }
	{ check<Mul> chk; chk.runFB(rlog); }
	{ check<Mul1> chk; chk.runFB(rlog); }
	{ check<CMul> chk; chk.runFB(rlog); }
	{ check<CMul1> chk; chk.runFB(rlog); }
	{ check<CMul2> chk; chk.runFB(rlog); }
	{ check<Mul2> chk; chk.runFB(rlog); }
	{ check<Mul3> chk; chk.runFB(rlog); }
	{ check<Div> chk; chk.runFB(rlog); }
	{ check<Div1> chk; chk.runFB(rlog); }
	{ check<CDiv> chk; chk.runFB(rlog); }
	{ check<CDiv1> chk; chk.runFB(rlog); }
	{ check<CDiv2> chk; chk.runFB(rlog); }
	{ check<Div2> chk; chk.runFB(rlog); }
	{ check<Div3> chk; chk.runFB(rlog); }
	{ check<Pow> chk; chk.runFB(rlog); }
	{ check<Pow2> chk; chk.runFB(rlog); }
	{ check<UAdd> chk; chk.runFB(rlog); }
	{ check<USub> chk; chk.runFB(rlog); }
	{ check<Sqr> chk; chk.runFB(rlog); }
	{ check<Exp> chk; chk.runFB(rlog); }
	{ check<Log> chk; chk.runFB(rlog); }
	{ check<Sqrt> chk; chk.runFB(rlog); }
	{ check<Sin> chk; chk.runFB(rlog); }
	{ check<Cos> chk; chk.runFB(rlog); }
	{ check<Tan> chk; chk.runFB(rlog); }
	{ check<ASin> chk; chk.runFB(rlog); }
	{ check<ACos> chk; chk.runFB(rlog); }
	{ check<ATan> chk; chk.runFB(rlog); }
	{ check<Nd> chk; chk.runFB(rlog); }
	{ check<Cnd> chk; chk.runFB(rlog); }
	{ check<Icnd> chk; chk.runFB(rlog); }
	{ check<csmap> chk; chk.runFB(rlog); }
	{ check<func1> chk; chk.runFB(rlog); }

	{ check<Add> chk; chk.runFFB(rlog); }
	{ check<Add1> chk; chk.runFFB(rlog); }
	{ check<CAdd> chk; chk.runFFB(rlog); }
	{ check<CAdd1> chk; chk.runFFB(rlog); }
	{ check<CAdd2> chk; chk.runFFB(rlog); }
	{ check<Add1> chk; chk.runFFB(rlog); }
	{ check<Add2> chk; chk.runFFB(rlog); }
	{ check<Sub> chk; chk.runFFB(rlog); }
	{ check<Sub1> chk; chk.runFFB(rlog); }
	{ check<CSub> chk; chk.runFFB(rlog); }
	{ check<CSub1> chk; chk.runFFB(rlog); }
	{ check<CSub2> chk; chk.runFFB(rlog); }
	{ check<Sub1> chk; chk.runFFB(rlog); }
	{ check<Sub2> chk; chk.runFFB(rlog); }
	{ check<Mul> chk; chk.runFFB(rlog); }
	{ check<Mul1> chk; chk.runFFB(rlog); }
	{ check<CMul> chk; chk.runFFB(rlog); }
	{ check<CMul1> chk; chk.runFFB(rlog); }
	{ check<CMul2> chk; chk.runFFB(rlog); }
	{ check<Mul1> chk; chk.runFFB(rlog); }
	{ check<Mul2> chk; chk.runFFB(rlog); }
	{ check<Div> chk; chk.runFFB(rlog); }
	{ check<Div1> chk; chk.runFFB(rlog); }
	{ check<CDiv> chk; chk.runFFB(rlog); }
	{ check<CDiv1> chk; chk.runFFB(rlog); }
	{ check<CDiv2> chk; chk.runFFB(rlog); }
	{ check<Div1> chk; chk.runFFB(rlog); }
	{ check<Div2> chk; chk.runFFB(rlog); }
	{ check<Pow> chk; chk.runFFB(rlog); }
	{ check<Pow2> chk; chk.runFFB(rlog); }
	{ check<UAdd> chk; chk.runFB(rlog); }
	{ check<USub> chk; chk.runFB(rlog); }
	{ check<Sqr> chk; chk.runFFB(rlog); }
	{ check<Exp> chk; chk.runFFB(rlog); }
	{ check<Log> chk; chk.runFFB(rlog); }
	{ check<Sqrt> chk; chk.runFFB(rlog); }
	{ check<Sin> chk; chk.runFFB(rlog); }
	{ check<Cos> chk; chk.runFFB(rlog); }
	{ check<Tan> chk; chk.runFFB(rlog); }
	{ check<ASin> chk; chk.runFFB(rlog); }
	{ check<ACos> chk; chk.runFFB(rlog); }
	{ check<ATan> chk; chk.runFFB(rlog); }
	{ check<Nd> chk; chk.runFFB(rlog); }
	{ check<Cnd> chk; chk.runFFB(rlog); }
	{ check<Icnd> chk; chk.runFFB(rlog); }
	{ check<csmap> chk; chk.runFFB(rlog); }
	{ check<func1> chk; chk.runFFB(rlog); }
}

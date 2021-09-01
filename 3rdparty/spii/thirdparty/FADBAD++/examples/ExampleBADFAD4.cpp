#include "badiff.h"
#include "fadiff.h"
#include <iostream>
using namespace std;
using namespace fadbad;

class Func
{
	public:
	template <class T>
	T operator()(const T& x, const T& y)
	{
		T z=sqrt(x);
		return y*z+sin(z);
	}
};

template <class C>
class FDiff
{
	public:
	template <class T>
	T operator()(
		T& o_dfdx, T& o_dfdy,
		const T& i_x, const T& i_y)
	{
		F<T> x(i_x),y(i_y); // Initialize arguments
		x.diff(0,2);        // Differentiate wrt. x
		y.diff(1,2);        // Differentiate wrt. y
		C func;             // Instantiate functor
		F<T> f(func(x,y));  // Evaluate function and record DAG
		o_dfdx=f.d(0);      // Value of df/dx
		o_dfdy=f.d(1);      // Value of df/dy
		return f.x();       // Return function value
	}
};

template <class C>
class BDiff
{
	public:
	template <class T>
	T operator()(
		T& o_dfdx, T& o_dfdy,
		const T& i_x, const T& i_y)
	{
		B<T> x(i_x),y(i_y); // Initialize arguments
		C func;             // Instantiate functor
		B<T> f(func(x,y));  // Evaluate function and record DAG
		f.diff(0,1);        // Differentiate
		o_dfdx=x.d(0);      // Value of df/dx
		o_dfdy=y.d(0);      // Value of df/dy
		return f.x();       // Return function value
	}
};

int main()
{
	double x,y,f,dfdx,dfdy;    // Declare variables
	x=1;                       // Initialize variable x
	y=2;                       // Initialize variable y
	FDiff<Func> FFunc;         // Functor for function and derivatives
	f=FFunc(dfdx,dfdy,x,y);    // Evaluate function and derivatives

	cout << "f(x,y)=" << f << endl;
	cout << "df/dx(x,y)=" << dfdx << endl;
	cout << "df/dy(x,y)=" << dfdy << endl;

	BDiff<Func> BFunc;         // Functor for function and derivatives
	f=BFunc(dfdx,dfdy,x,y);    // Evaluate function and derivatives

	cout << "f(x,y)=" << f << endl;
	cout << "df/dx(x,y)=" << dfdx << endl;
	cout << "df/dy(x,y)=" << dfdy << endl;

	return 0;
}

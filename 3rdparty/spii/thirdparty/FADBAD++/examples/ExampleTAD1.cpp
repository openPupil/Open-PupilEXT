#include "tadiff.h"
#include <iostream>
using namespace std;
using namespace fadbad;

T<double> func(const T<double>& x, const T<double>& y)
{
	T<double> z=sqrt(x);
	return y*z+sin(z);
}

int main()
{
	T<double> x,y,f;     // Declare variables x,y,f
	x=1;                 // Initialize variable x
	y=2;                 // Initialize variable y
	x[1]=1;              // Taylor-expand wrt. x (dx/dx=1)
	f=func(x,y);         // Evaluate function and record DAG
	double fval=f[0];    // Value of function
	f.eval(10);          // Taylor-expand f to degree 10
	// f[0]...f[10] now contains the Taylor-coefficients.
	cout << "f(x,y)=" << fval << endl;
	for(int i=0;i<=10;i++)
	{
		double c=f[i];// The i'th taylor coefficient
		cout << "(1/k!)*(d^"<<i<<"f/dx^" << i << ")=" << c << endl;
	}

	f.reset();           // Reset the values in the DAG
	x[0]=3;              // New value for x
	y[0]=4;              // New value for y
	y[1]=1;              // Taylor-expand wrt. y (dy/dy=1)
	f.eval(10);          // Taylor-expand f to degree 10
	// f[0]...f[10] now contains the Taylor-coefficients.
	for(int i=0;i<=10;i++)
	{
		double c=f[i];// The i'th taylor coefficient
		cout << "(1/k!)*(d^"<<i<<"f/dy^" << i << ")=" << c << endl;
	}

	return 0;
}

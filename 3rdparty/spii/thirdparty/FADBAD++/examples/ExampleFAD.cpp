#include "fadiff.h"
#include <iostream>
using namespace std;
using namespace fadbad;

F<double> func(const F<double>& x, const F<double>& y)
{
	F<double> z=sqrt(x);
	return y*z+sin(z);
}

int main()
{
	F<double> x,y,f;     // Declare variables x,y,f
	x=1;                 // Initialize variable x
	x.diff(0,2);         // Differentiate with respect to x (index 0 of 2)
	y=2;                 // Initialize variable y
	y.diff(1,2);         // Differentiate with respect to y (index 1 of 2)
	f=func(x,y);         // Evaluate function and derivatives
	double fval=f.x();   // Value of function
	double dfdx=f.d(0);  // Value of df/dx (index 0 of 2)
	double dfdy=f.d(1);  // Value of df/dy (index 1 of 2)

	cout << "f(x,y)=" << fval << endl;
	cout << "df/dx(x,y)=" << dfdx << endl;
	cout << "df/dy(x,y)=" << dfdy << endl;

	return 0;
}

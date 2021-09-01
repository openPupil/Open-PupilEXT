#include "badiff.h"
#include <iostream>
using namespace std;
using namespace fadbad;

B<double> func(const B<double>& x, const B<double>& y)
{
	B<double> z=sqrt(x);
	return y*z+sin(z);
}

int main()
{
	B<double> x,y,f;    // Declare variables x,y,f
	x=1;                // Initialize variable x
	y=2;                // Initialize variable y
	f=func(x,y);        // Evaluate function and record DAG
	f.diff(0,1);        // Differentiate f (index 0 of 1)
	double fval=f.x();  // Value of function
	double dfdx=x.d(0); // Value of df/dx (index 0 of 1)
	double dfdy=y.d(0); // Value of df/dy (index 0 of 1)

	cout << "f(x,y)=" << fval << endl;
	cout << "df/dx(x,y)=" << dfdx << endl;
	cout << "df/dy(x,y)=" << dfdy << endl;

	return 0;
}

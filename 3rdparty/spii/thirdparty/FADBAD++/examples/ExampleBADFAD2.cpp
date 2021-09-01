#include "badiff.h"
#include "fadiff.h"
#include <iostream>
using namespace std;
using namespace fadbad;

B< F<double> > func(const B< F<double> >& x, const B< F<double> >& y)
{
	B< F<double> > z=sqrt(x);
	return y*z+sin(z);
}

F<double> dfunc(
	F<double>& o_dfdx, F<double>& o_dfdy,
	const F<double>& i_x, const F<double>& i_y)
{
	B< F<double> > x(i_x),y(i_y);   // Initialize arguments
	B< F<double> > f(func(x,y));    // Evaluate function and record DAG
	f.diff(0,1);                    // Differentiate
	o_dfdx=x.d(0);                  // Value of df/dx
	o_dfdy=y.d(0);                  // Value of df/dy
	return f.x();                   // Return function value
}

double ddfunc(
	double& o_dfdxdx, double& o_dfdxdy,
	double& o_dfdydx, double& o_dfdydy,
	double& o_dfdx, double& o_dfdy,
	const double& i_x, const double& i_y)
{
	F<double> x(i_x),y(i_y),dfdx,dfdy;// Initialize arguments
	x.diff(0,2);                    // Second order wrt. x
	y.diff(1,2);                    // Second order wrt. y
	F<double> f(dfunc(dfdx,dfdy,x,y));// Evaluate function and derivatives
	o_dfdxdx=dfdx.d(0);             // Value of df/dx
	o_dfdxdy=dfdx.d(1);             // Value of df/dxdy
	o_dfdydx=dfdy.d(0);             // Value of df/dx
	o_dfdydy=dfdy.d(1);             // Value of df/dxdy
	o_dfdx=dfdx.x();                // Value of df/dx
	o_dfdy=dfdy.x();                // Value of df/dx
	return f.x();                   // Return function value
}

int main()
{
	double x,y,f,dfdx,dfdy,
		dfdxdx,dfdxdy,
		dfdydx,dfdydy;     // Declare variables
	x=1;                       // Initialize variable x
	y=2;                       // Initialize variable y
	f=ddfunc(dfdxdx,dfdxdy,
		dfdydx,dfdydy,
		dfdx,dfdy,x,y);    // Evaluate function and derivatives

	cout << "f(x,y)=" << f << endl;
	cout << "df/dx(x,y)=" << dfdx << endl;
	cout << "df/dy(x,y)=" << dfdy << endl;
	cout << "df/dx(x,y)=" << dfdxdx << endl;
	cout << "df/dxdy(x,y)=" << dfdxdy << endl;
	cout << "df/dydx(x,y)=" << dfdydx << endl;
	cout << "df/dy(x,y)=" << dfdydy << endl;

	return 0;
}

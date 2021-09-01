#include "tadiff.h"
#include <iostream>
using namespace std;
using namespace fadbad;

double s(10), r(28), q(8/3);

class Lorenz
{
public:
	T<double> x,y,z,p;           // Independent variables
	T<double> xp,yp,zp,pp;       // Dependent variables
	Lorenz()
	{
	// record DAG at construction:
		xp = p*(s*(y-x));
		yp = p*(x*(r-z)-y);
		zp = p*(x*y-q*z);
		pp = double(0);
	}
	void reset()
	{
		xp.reset();
		yp.reset();
		zp.reset();
		pp.reset();
	}
};

int main()
{
	// Construct ODE:
	Lorenz ode;

	// Set point of expansion:
	ode.x[0]=2.14736765;
	ode.y[0]=-2.07804819;
	ode.z[0]=r-1;
	ode.p[0]=1.55865218;

	int i,j;
	for(i=0;i<10;i++)
	{
	// Evaluate the i'th Taylor coefficient of
	// the r.h.s. of the ODE:
		ode.xp.eval(i);
		ode.yp.eval(i);
		ode.zp.eval(i);
		ode.zp.eval(i);

	// Since d(x,y,z,p)/dt=lorenz(x,y,z,p) we have
		ode.x[i+1]=ode.xp[i]/double(i+1);
		ode.y[i+1]=ode.yp[i]/double(i+1);
		ode.z[i+1]=ode.zp[i]/double(i+1);
		ode.p[i+1]=ode.pp[i]/double(i+1);
	}

	// Print out the Taylor coefficients for the solution
	// of the ODE:
	for(i=0;i<=10;i++)
	{
		cout << "x[" << i << "]=" << ode.x[i] << endl;
		cout << "y[" << i << "]=" << ode.y[i] << endl;
		cout << "z[" << i << "]=" << ode.z[i] << endl;
		cout << "p[" << i << "]=" << ode.p[i] << endl;
	}

	return 0;
}

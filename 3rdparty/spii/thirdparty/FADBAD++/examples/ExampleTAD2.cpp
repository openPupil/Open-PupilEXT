#include "tadiff.h"
#include <iostream>
using namespace std;
using namespace fadbad;

class TODE
{
public:
	T<double> x;            // Independent variables
	T<double> xp;           // Dependent variables
	TODE(){xp=cos(x);}      // record DAG at construction
};

int main()
{
	TODE ode;                // Construct ODE:
	ode.x[0]=1;              // Set point of expansion:
	for(int i=0;i<10;i++)
	{
		ode.xp.eval(i); // Evaluate i'th Taylor coefficient
		ode.x[i+1]=ode.xp[i]/double(i+1);// Use dx/dt=ode(x).
	}
	// ode.x[0]...ode.x[10] now contains the Taylor-coefficients
	// of the solution of the ODE.

	// Print out the Taylor coefficients for the solution
	// of the ODE:
	for(int i=0;i<=10;i++)
	{
		cout << "x[" << i << "]=" << ode.x[i] << endl;
	}

	return 0;
}

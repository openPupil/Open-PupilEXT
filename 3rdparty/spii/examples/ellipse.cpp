// Petter Strandmark 2012.
//
// This example illustrates how a change of variables
// can be used to compute the closest point lying on
// an ellipse to a given point.

#include <iostream>
#include <stdexcept>
using namespace std;

#include <spii/auto_diff_term.h>
#include <spii/solver.h>

using namespace spii;

// Simple term equal to the squared distance
// to (x0, y0).
class Distance
{
public:
	Distance(double x0, double y0)
	{
		this->x0 = x0;
		this->y0 = y0;
	}

	template<typename R>
	R operator()(const R* const xy) const
	{
		R dx = xy[0] - x0;
		R dy = xy[1] - y0;
		return dx*dx + dy*dy;
	}
private:
	double x0, y0;
};

// Change of variable forcing (x, y) to lie on an
// ellipse parametrized by 5 variables.
class Ellipse
{
public:
	Ellipse(double x0, double y0, double a,
	        double b, double phi)
	{
		this->x0 = x0;
		this->y0 = y0;
		this->a = a;
		this->b = b;
		this->phi = phi;
	}

	// Compute (x, y) given the single parameter t.
	template<typename R>
	void t_to_x(R* xy, const R* t) const
	{
		xy[0] = x0 + a * cos(*t)*cos(phi) - b * sin(*t)*sin(phi);
		xy[1] = y0 + a * cos(*t)*sin(phi) + b * sin(*t)*cos(phi);
	}

	// This function is supposed to compute t given
	// x and y. The only time the solver actually uses 
	// this function is when computing the starting
	// point t0 given x and y. If this functionality
	// is not required, the implementation can be
	// omitted.
	template<typename R>
	void x_to_t(R* t, const R* x) const
	{
		// This will be the starting value of t.
		t[0] = 0;
	}

	int x_dimension() const
	{
		return 2;
	}

	int t_dimension() const
	{
		return 1;
	}

private:
	double x0, y0, a, b, phi;
};


int main_function()
{
	// Ellipse
	double x0 = 3;
	double y0 = 4;
	double a = 3;
	double b = 0.5;
	double phi = 1.0;

	Function f;

	// Add (x, y) as a variable, constrained to lie on the ellipse.
	vector<double> xy(2);
	f.add_variable_with_change<Ellipse>(&xy[0], 2, x0, y0, a, b, phi);

	// Add a term measuring the distance to (6, 6).
	f.add_term(make_shared<AutoDiffTerm<Distance, 2>>(6.0, 6.0),
	           xy.data());

	LBFGSSolver solver;
	SolverResults results;
	solver.solve(f, &results);

	cout << results << endl;
	cout << "Final point: (" << xy[0] << ", " << xy[1] << ")\n";
	cout << "Evaluate: " << f.evaluate() << endl;
	return 0;
}

int main()
{
	try {
		return main_function();
	}
	catch (exception& e) {
		cerr << "Exception: " << e.what() << endl;
	}
}

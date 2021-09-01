// Petter Strandmark 2013.
//
// This example illustrates how a constraint can
// be used to compute the closest point lying on
// an ellipse to a given point.
//
// Compare to the other ellipse example, which
// uses an explicit parametrization and is more
// efficient.

#include <iostream>
#include <stdexcept>
using namespace std;

#include <spii/auto_diff_term.h>
#include <spii/constrained_function.h>
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

// Constraint forcing (x, y) to lie on an
// ellipse parametrized by 5 variables.
class Ellipse
{
public:
	Ellipse(double x0, double y0, double a,
	        double b, double phi)
	{
		// Parametric → Foci/String
		// http://www.cs.cornell.edu/cv/OtherPdf/Ellipse.pdf
		check(a*a >= b*b, "Ellipse requires that a^2 >= b^2.");

		double c = sqrt(a*a - b*b);
		x1 = x0 - cos(phi)*c;
		y1 = y0 - sin(phi)*c;
		x2 = x0 + cos(phi)*c;
		y2 = y0 + sin(phi)*c;
		s = 2*a;
	}

	template<typename R>
	R operator()(R* xy) const
	{
		const R& x = xy[0];
		const R& y = xy[1];
		return sqrt((x - x1)*(x - x1) + (y - y1)*(y - y1)) + sqrt((x - x2)*(x - x2) + (y - y2)*(y - y2)) - s;
	}

private:
	double x1, y1;
	double x2, y2;
	double s;
};


int main_function()
{
	// Ellipse
	double x0 = 3;
	double y0 = 4;
	double a = 3;
	double b = 0.5;
	double phi = 1.0;

	ConstrainedFunction f;
	vector<double> xy(2);

	// Add a term measuring the distance to (6, 6).
	f.add_term(make_shared<AutoDiffTerm<Distance, 2>>(6.0, 6.0),
	           xy.data());

	// Constrain xy to lie on an ellipse.
	auto ellipse = make_shared<AutoDiffTerm<Ellipse, 2>>(x0, y0, a, b, phi);
	f.add_equality_constraint_term("Ellipse", ellipse, xy.data());

	LBFGSSolver solver;
	SolverResults results;
	f.solve(solver, &results);

	cout << results << endl;
	cout << "Final point: (" << xy[0] << ", " << xy[1] << ")\n";
	cout << "Evaluate: " << f.objective().evaluate() << endl;
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

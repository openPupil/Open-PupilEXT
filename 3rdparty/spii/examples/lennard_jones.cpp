// Petter Strandmark 2012.
//
// See http://doye.chem.ox.ac.uk/jon/structures/LJ/tables.150.html
// for best known minima for N <= 150.
//

#include <functional>
#include <iomanip>
#include <iostream>
#include <random>

#include <spii/auto_diff_term.h>
#include <spii/solver.h>
#include <spii/solver-callbacks.h>

using namespace spii;

struct LennardJonesTerm
{
	template<typename R>
	R operator()(const R* const p1, const R* const p2) const
	{
		R dx = p1[0] - p2[0];
		R dy = p1[1] - p2[1];
		R dz = p1[2] - p2[2];
		R r2 = dx*dx + dy*dy + dz*dz;
		R r6  = r2*r2*r2;
		R r12 = r6*r6;
		return 1.0 / r12 - 2.0 / r6;
	}
};

int main()
{
	using namespace std;

	mt19937 prng(1);
	normal_distribution<double> normal;
	auto randn = bind(normal, ref(prng));

	int N = -1;
	cout << "Enter N = " << endl;
	cin >> N;

	Function potential;
	vector<Eigen::Vector3d> points(N);

	int n = int(ceil(pow(double(N), 1.0/3.0)));

	// Initial position is a cubic grid with random pertubations.
	for (int i = 0; i < N; ++i) {
		int x =  i % n;
		int y = (i / n) % n;
		int z = (i / n) / n;

		potential.add_variable(&points[i][0], 3);
		points[i][0] = x + 0.05 * randn();
		points[i][1] = y + 0.05 * randn();
		points[i][2] = z + 0.05 * randn();
	}

	for (int i = 0; i < N; ++i) {
		for (int j = i + 1; j < N; ++j) {
			potential.add_term<AutoDiffTerm<LennardJonesTerm, 3, 3>>(
				&points[i][0],
				&points[j][0]);
		}
	}

	LBFGSSolver solver;
	//solver.sparsity_mode = Solver::DENSE;  // For NewtonSolver.
	solver.maximum_iterations = 3000;
	ofstream file("convergence.data");
	FileCallback callback(file);
	solver.callback_function = callback;

	SolverResults results;
	solver.solve(potential, &results);

	cout << results;
	potential.print_timing_information(cout);

	cout << "Energy = " << setprecision(10) << potential.evaluate() << endl;
}

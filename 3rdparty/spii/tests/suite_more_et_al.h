// Include this function in a file defining the run_test
// template function.
//
// Petter Strandmark 2012
//


// ----------------------------------------------------------------------
// Test functions from
// Jorge J. More, Burton S. Garbow and Kenneth E. Hillstrom,
// "Testing unconstrained optimization software",
// Transactions on Mathematical Software 7(1):17-41, 1981.
// http://www.caam.rice.edu/~zhang/caam454/nls/MGH.pdf
//
struct Rosenbrock
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 =  x[1] - x[0]*x[0];
		R d1 =  1 - x[0];
		return 100 * d0*d0 + d1*d1;
	}
};

TEST(Solver, Rosenbrock)
{
	double x[2] = {-1.2, 1.0};
	double fval = run_test<Rosenbrock, 2>(x);

	EXPECT_LT( std::fabs(x[0] - 1.0), 1e-9);
	EXPECT_LT( std::fabs(x[1] - 1.0), 1e-9);
	EXPECT_LT( std::fabs(fval), 1e-9);
}

TEST(Solver, RosenbrockFar)
{
	double x[2] = {-1e6, 1e6};

	auto solver = create_solver();
	solver->gradient_tolerance = 1e-40;
	solver->maximum_iterations = 100000;
	double fval = run_test<Rosenbrock, 2>(x, solver.get());

	EXPECT_LT( std::fabs(x[0] - 1.0), 1e-9);
	EXPECT_LT( std::fabs(x[1] - 1.0), 1e-9);
	EXPECT_LT( std::fabs(fval), 1e-9);
}

struct FreudenStein_Roth
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 =  -13.0 + x[0] + ((5.0 - x[1])*x[1] - 2.0)*x[1];
		R d1 =  -29.0 + x[0] + ((x[1] + 1.0)*x[1] - 14.0)*x[1];
		return d0*d0 + d1*d1;
	}
};

TEST(Solver, FreudenStein_Roth)
{
	double x[2] = {0.5, -2.0};
	run_test<FreudenStein_Roth, 2>(x);

	// Can end up in local minima 48.9842...
	//EXPECT_LT( std::fabs(x[0] - 5.0), 1e-9);
	//EXPECT_LT( std::fabs(x[1] - 4.0), 1e-9);
	//EXPECT_LT( std::fabs(f.evaluate()), 1e-9);
}

struct Powell_badly_scaled
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 = 1e4*x[0]*x[1] - 1;
		R d1 = exp(-x[0]) + exp(-x[1]) - 1.0001;
		return d0*d0 + d1*d1;
	}
};

TEST(Solver, Powell_badly_scaled)
{
	double x[2] = {0.0, 1.0};
	double fval = run_test<Powell_badly_scaled, 2>(x);

	EXPECT_LT( std::fabs(fval), 1e-9);
}

struct Brown_badly_scaled
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 = x[0] - 1e6;
		R d1 = x[1] - 2e-6;
		R d2 = x[0]*x[1] - 2;
		return d0*d0 + d1*d1 + d2*d2;
	}
};

TEST(Solver, Brown_badly_scaled)
{
	double x[2] = {1.0, 1.0};
	double fval = run_test<Brown_badly_scaled, 2>(x);

	EXPECT_LT( std::fabs(x[0] - 1e6),  1e-3);
	EXPECT_LT( std::fabs(x[1] - 2e-6), 1e-9);
	EXPECT_LT( std::fabs(fval), 1e-9);
}


struct Beale
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 = 1.5   - x[0] * (1.0 - x[1]);
		R d1 = 2.25  - x[0] * (1.0 - x[1]*x[1]);
		R d2 = 2.625 - x[0] * (1.0 - x[1]*x[1]*x[1]);
		return d0*d0 + d1*d1 + d2*d2;
	}
};

TEST(Solver, Beale)
{
	double x[2] = {1.0, 1.0};
	double fval = run_test<Beale, 2>(x);

	EXPECT_LT( std::fabs(x[0] - 3.0),  1e-3);
	EXPECT_LT( std::fabs(x[1] - 0.5), 1e-9);
	EXPECT_LT( std::fabs(fval), 1e-9);
}


struct JennrichSampson
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R fval = 0;
		for (int ii = 1; ii <= 10; ++ii) {
			double i = ii;
			R d = 2.0 + 2.0*i - (exp(i*x[0]) + exp(i*x[1]));
			fval += d*d;
		}
		return fval;
	}
};

TEST(Solver, JennrichSampson)
{
	double x[2] = {0.3, 0.4};
	double fval = run_test<JennrichSampson, 2>(x);

	EXPECT_LT( std::fabs(fval - 124.362), 0.001);
}

struct HelicalValley
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R theta = 1.0 / (2.0 * 3.141592653589793)
		          * atan(x[1] / x[0]);
		if (x[0] < 0) {
			theta += 0.5;
		}
		R d0 = 10.0 * (x[2] - 10.0 * theta);
		R d1 = 10.0 * (sqrt(x[0]*x[0] + x[1]*x[1]) - 1.0);
		R d2 = x[2];
		return d0*d0 + d1*d1 + d2*d2;
	}
};

TEST(Solver, HelicalValley)
{
	double x[3] = {-1.0, 0.0, 0.0};
	double fval = run_test<HelicalValley, 3>(x);

	EXPECT_LT( std::fabs(x[0] - 1.0),  1e-9);
	EXPECT_LT( std::fabs(x[1]), 1e-9);
	EXPECT_LT( std::fabs(x[2]), 1e-9);
	EXPECT_LT( std::fabs(fval), 1e-9);
}

struct Bard
{
	template<typename R>
	R operator()(const R* const x) const
	{
		double y[15] = {0.14, 0.18, 0.22, 0.25,
		                0.29, 0.32, 0.35, 0.39,
		                0.37, 0.58, 0.73, 0.96,
						1.34, 2.10, 4.39};
		R fval = 0;
		for (int ii = 1; ii <= 15; ++ii) {
			double u = ii;
			double v = 16.0 - ii;
			double w = std::min(u, v);
			R d = y[ii - 1] - (x[0] + u / (v * x[1] + w * x[2]));
			fval += d*d;
		}
		return fval;
	}
};

TEST(Solver, Bard)
{
	double x[3] = {1.0, 1.0, 1.0};
	double fval = run_test<Bard, 3>(x);

	EXPECT_LT( std::fabs(fval - 8.21487e-3), 1e-7);
}

struct Gaussian
{
	template<typename R>
	R operator()(const R* const x) const
	{
		double y[15] = {0.0009, 0.0044, 0.0175, 0.0540,
		                0.1295, 0.2420, 0.3521, 0.3989,
		                0.3521, 0.2420, 0.1295, 0.0540,
		                0.0175, 0.0044, 0.0009};
		R fval = 0;
		for (int ii = 1; ii <= 15; ++ii) {
			double t = (8.0 - ii) / 2.0;
			R tdiff = t - x[2];
			R d = x[0] * exp( -x[1]*tdiff*tdiff / 2.0) - y[ii - 1];
			fval += d*d;
		}
		return fval;
	}
};

TEST(Solver, Gaussian)
{
	double x[3] = {0.4, 1.0, 0.0};
	double fval = run_test<Gaussian, 3>(x);

	EXPECT_LT( std::fabs(fval - 1.12793e-8), 1e-12);
}

struct Meyer
{
	template<typename R>
	R operator()(const R* const x) const
	{
		double y[16] = {34780, 28610, 23650, 19630,
		                16370, 13720, 11540,  9744,
		                 8261,  7030,  6005,  5147,
		                 4427,  3820,  3307,  2872};
		R fval = 0;
		for (int ii = 1; ii <= 16; ++ii) {
			double t = 45.0 + 5.0 * ii;
			R d = x[0] * exp( x[1] / (t + x[2])) - y[ii - 1];
			fval += d*d;
		}
		return fval;
	}
};

TEST(Solver, Meyer)
{
	double x[3] = {0.02, 4000.0, 250.0};
	auto solver = create_solver();
	if (solver->maximum_iterations < 500) {
		solver->maximum_iterations = 500;
	}
	double fval = run_test<Meyer, 3>(x, solver.get());

	EXPECT_LT( std::fabs(fval - 87.9458), 1e-3);
}

template<int m = 10>
struct Gulf
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R fval = 0;
		for (int ii = 1; ii <= m; ++ii) {
			double t = ii / 100.0;
			double y = 25.0 + pow(-50.0 * log(t), 2.0 / 3.0);
			R d = exp( - pow( y * m * ii * x[1], x[2]) / x[0] ) - t;
			fval += d*d;
		}
		return fval;
	}
};

TEST(Solver, Gulf)
{
	double x[3] = {5, 2.5, 0.15};
	run_test<Gulf<3>, 3>(x);

	// The Gulf function does not evaluate to close to 0
	// at the globally optimal point. Hence these tests
	// are disabled.
	//EXPECT_LT( std::fabs(x[0] - 50.0), 1e-9);
	//EXPECT_LT( std::fabs(x[1] - 25.0), 1e-9);
	//EXPECT_LT( std::fabs(x[2] - 1.5),  1e-9);
	//EXPECT_LT( std::fabs(fval), 1e-9);
}

template<int m>
struct Box
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R fval = 0;
		for (int ii = 1; ii <= m; ++ii) {
			double t = 0.1 * ii;
			R d = exp(-t*x[0]) - exp(-t*x[1]) - x[2]*(exp(-t) - exp(-10.0*t));
			fval += d*d;
		}
		return fval;
	}
};

TEST(Solver, Box)
{
	double x[3] = {1.0, 10.0, 20.0};
	double fval = run_test<Box<10>, 3>(x);

	EXPECT_LT( std::fabs(fval), 1e-9);
}

struct PowellSingular
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R d0 = x[0] + 10 * x[1];
		R d1 = sqrt(5.0) * (x[2] - x[3]);
		R d2 = x[1] - 2.0*x[3];
		d2 = d2 * d2;
		R d3 = sqrt(10.0) * (x[0] - x[3]) * (x[0] - x[3]);
		return d0*d0 + d1*d1 + d2*d2 + d3*d3;
	}
};

TEST(Solver, PowellSingular)
{
	double x[4] = {3.0, -1.0, 0.0, 1.0};
	double fval = run_test<PowellSingular, 4>(x);

	EXPECT_LT( std::fabs(x[0]), 1e-3);  // Hard to end up with the variables
	EXPECT_LT( std::fabs(x[1]), 1e-3);  // very close to the origin.
	EXPECT_LT( std::fabs(x[2]), 1e-3);
	EXPECT_LT( std::fabs(x[3]), 1e-3);
	EXPECT_LT( std::fabs(fval), 1e-12);
}

struct Wood
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R f1 = 10.0 * (x[1] - x[0]*x[0]);
		R f2 = 1 - x[0];
		R f3 = sqrt(90.0) * (x[3] - x[2]*x[2]);
		R f4 = 1- x[2];
		R f5 = sqrt(10.0) * (x[1] + x[3] - 2.0);
		R f6 = 1.0 / sqrt(10.0) * (x[1] - x[3]);
		return f1*f1 + f2*f2 + f3*f3 + f4*f4 + f5*f5 + f6*f6;
	}
};

TEST(Solver, Wood)
{
	double x[4] = {-3.0, -1.0, -3.0, -1.0};
	double fval = run_test<Wood, 4>(x);

	EXPECT_LT( std::fabs(x[0] - 1.0), 1e-8);
	EXPECT_LT( std::fabs(x[1] - 1.0), 1e-8);
	EXPECT_LT( std::fabs(x[2] - 1.0), 1e-8);
	EXPECT_LT( std::fabs(x[3] - 1.0), 1e-8);
	EXPECT_LT( std::fabs(fval), 1e-8);
}

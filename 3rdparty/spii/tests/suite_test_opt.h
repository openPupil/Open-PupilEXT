// Include this function in a file defining the run_test
// template function.
//
// Petter Strandmark 2012
//

//---------------------------------------------------------------
//
// Test functions from TEST_OPT
// http://people.sc.fsu.edu/~jburkardt/m_src/test_opt/test_opt.html

// #28
struct SchafferFunctionF7
{
	template<typename R>
	R operator()(const R* const x) const
	{
		using std::sqrt;
		using std::sin;
		using std::pow;

		R r = sqrt (x[0]*x[0] + x[1]*x[1]);
		R s = sin (R(50.0) * pow(r, 0.2));
		return sqrt (r) * (R(1.0) + s*s);
	}
};

TEST(Solver, SchafferFunctionF7)
{
	double x[2] = {-5.0, 10.0};
	run_test<SchafferFunctionF7, 2>(x);
	// Global minimum is 0,0.
}

// #29
struct GoldsteinPricePolynomial
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R a = x[0] + x[1] + 1.0;

		R b = 19.0 - 14.0 * x[0] + 3.0 * x[0] * x[0] - 14.0 * x[1]
			+ 6.0 * x[0] * x[1] + 3.0 * x[1] * x[1];

		R c = 2.0 * x[0] - 3.0 * x[1];

		R d = 18.0 - 32.0 * x[0] + 12.0 * x[0] * x[0] + 48.0 * x[1]
			- 36.0 * x[0] * x[1] + 27.0 * x[1] * x[1];

		return ( 1.0 + a * a * b ) * ( 30.0 + c * c * d );
	}
};

TEST(Solver, GoldsteinPricePolynomial)
{
	double x[2] = {-0.5, 0.25};
	// Only expect a local minimum where the gradient
	// is small.
	auto solver = create_solver();
	solver->argument_improvement_tolerance = 0;
	solver->function_improvement_tolerance = 0;
	run_test<GoldsteinPricePolynomial, 2>(x, solver.get());
}

// #30
struct BraninRCOS
{
	template<typename R>
	R operator()(const R* const x) const
	{
		const double pi = 3.141592653589793;
		const double a  = 1.0;
		const double d  = 6.0;
		const double e  = 10.0;
		const double b  = 5.1 / ( 4.0 * pi*pi );
		const double c  = 5.0 / pi;
		const double ff = 1.0 / ( 8.0 * pi );

		R expr = ( x[1] - b * x[0]*x[0] + c * x[0] - d );
		return a * expr * expr
			+ e * ( 1.0 - ff ) * cos ( x[0] ) + e;
	}
};

TEST(Solver, BraninRCOS)
{
	double x[2] = {-1.0, 1.0};
	// Only expect a local minimum where the gradient
	// is small.
	auto solver = create_solver();
	solver->argument_improvement_tolerance = 0;
	solver->function_improvement_tolerance = 0;
	solver->gradient_tolerance = 1e-10;
	run_test<BraninRCOS, 2>(x, solver.get());
}

// #34
struct SixHumpCamelBack
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return ( 4.0 - 2.1 * x[0]*x[0] + x[0]*x[0]*x[0]*x[0] / 3.0 ) * x[0]*x[0]
	            + x[0] * x[1] + 4.0 * ( x[1]*x[1] - 1.0 ) * x[1]*x[1];
	}
};

TEST(Solver, SixHumpCamelBack)
{
	double x[2] = {-1.5, 0.5};
	// Only expect a local minimum where the gradient
	// is small.
	auto solver = create_solver();
	solver->argument_improvement_tolerance = 0;
	solver->function_improvement_tolerance = 0;
	solver->gradient_tolerance = 1e-10;
	run_test<SixHumpCamelBack, 2>(x, solver.get());
}


// #35
struct Shubert
{
	template<typename R>
	R operator()(const R* const x) const
	{
		R factor1 = 0.0;
		for (int i = 1; i<= 5; ++i) {
			double y = i;
			factor1 += y * cos( (y + 1.0) * x[0] + y );
		}

		R factor2 = 0.0;
		for (int i = 1; i<= 5; ++i) {
			double y = i;
			factor2 += y * cos( (y + 1.0) * x[1] + y );
		}

		return factor1 * factor2;
	}
};

TEST(Solver, Shubert)
{
	// Slightly perturbed from {0.5, 1.0}.
	// Some solvers seem to have "bad luck" on this test case.
	double x[2] = {0.50001, 1.00001};
	auto solver = create_solver();
	run_test<Shubert, 2>(x, solver.get());
}

// #37
struct Easom
{
	template<typename R>
	R operator()(const R* const x) const
	{
		const double pi = 3.141592653589793;
		R arg = - (x[0] - pi)*(x[0] - pi) - (x[1] - pi)*(x[1] - pi);
		return -cos(x[0]) * cos(x[1]) * exp(arg);
	}
};

TEST(Solver, Easom)
{
	auto solver = create_solver();
	solver->maximum_iterations = 10000;

	double x[2] = {0.5, 1.0};
	run_test<Easom, 2>(x, solver.get());

	// There seems to be other local minima, though.
	//EXPECT_LT( std::fabs(x[0] - 3.141592653589793), 1e-8);
	//EXPECT_LT( std::fabs(x[1] - 3.141592653589793), 1e-8);
}

// #38
struct Bohachevsky1
{
	// f = x(1) * x(1) - 0.3 * cos ( 3.0 * pi * x(1) ) ...
	//   + 2.0 * x(2) * x(2) - 0.4 * cos ( 4.0 * pi * x(2) ) ...
	//   + 0.7;
	template<typename R>
	R operator()(const R* const x) const
	{
		return x[0] * x[0] - 0.3 * cos(3.0 * 3.141592 * x[0])
			+ 2.0 * x[1] * x[1] - 0.4 * cos(4.0 * 3.141592 * x[1])
			+ 0.7;
	}
};

// #39
struct Bohachevsky2
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return x[0]*x[0] + 2.0*x[1]*x[1]
			-0.3*cos(3.0 * 3.141592 * x[0]) *
				 cos( 4.0 * 3.141592 * x[1] ) + 0.3;
	}
};

// #40
struct Bohachevsky3
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return x[0]*x[0] + 2.0*x[1]*x[1]
			- 0.3 * cos ( 3.0 * 3.141592 * x[0] )
			+ cos ( 4.0 * 3.141592 * x[1] ) + 0.3;
	}
};

TEST(Solver, Bohachevsky)
{
	double x[2];

	// Only expect a local minimum where the gradient
	// is small.
	auto solver = create_solver();
	solver->argument_improvement_tolerance = 0;
	solver->function_improvement_tolerance = 0;


	if (solver->gradient_tolerance >= 1e-16) {
		solver->gradient_tolerance = 1e-8;
	}

	x[0] = 0.5;
	x[1] = 1.0;
	run_test<Bohachevsky1, 2>(x, solver.get());

	x[0] = 0.6;
	x[1] = 1.3;
	run_test<Bohachevsky2, 2>(x, solver.get());

	x[0] = 0.5;
	x[1] = 1.0;
	run_test<Bohachevsky3, 2>(x, solver.get());
}


// #41
struct Colville
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return 100.0 * (x[1] - x[0]*x[0]) * (x[1] - x[0]*x[0])
		+ (1.0 - x[0]) * (1.0 - x[0])
		+ 90.0 * (x[3] - x[2]*x[2]) * (x[3] - x[2]*x[2])
		+ (1.0 - x[2]) * (1.0 - x[2])
		+ 10.1 * ( (x[1] - 1.0 ) * (x[1] - 1.0)
			+ (x[3] - 1.0) * (x[3] - 1.0) )
		+ 19.8 * (x[1] - 1.0) * (x[3] - 1.0);
	}
};

TEST(Solver, Colville)
{
	double x[4] = {-0.5, 1.0, -0.5, -1.0};
	run_test<Colville, 4>(x);

	EXPECT_LT( std::fabs(x[0] - 1.0), 1e-8);
	EXPECT_LT( std::fabs(x[1] - 1.0), 1e-8);
	EXPECT_LT( std::fabs(x[2] - 1.0), 1e-8);
	EXPECT_LT( std::fabs(x[3] - 1.0), 1e-8);
}

// #42
struct Powell3D
{
	template<typename R>
	R operator()(R* x) const
	{
		R term = 0.0;
		if (to_double(x[1]) != 0.0) {
			R arg = (x[0] + 2.0*x[1] + x[2]) / x[1];
			term = exp(-arg*arg);
		}

		return 3.0
			- 1.0 / (1.0 + (x[0] - x[1])*(x[0] - x[1]))
			- sin( 0.5 * 3.141592653589793 * x[1] * x[2])
			- term;
	}
};

TEST(Solver, Powell3D)
{
	// There appears to be numerical problems at
	// x = (-1, -1, 3).
	//
	// Symbolic computation shows that the gradient indeed is 0
	// at (-1, -1, 3), but the function returns a gradient whose
	// maximum element is over 1e-8.
	//

	double x[3] = {-1.0, -1.0, 3.0};
	Function f;
	f.add_variable(x, 3);
	f.add_term(std::make_shared<AutoDiffTerm<Powell3D, 3>>(), x);
	Eigen::VectorXd xvec(3);

	xvec[0] = -1.0;
	xvec[1] = -1.0;
	xvec[2] =  3.0;
	Eigen::VectorXd g;
	f.evaluate(xvec, &g);
	INFO("g(-1, -1, 3)  = (" << g.transpose() << ")");

	x[0] = 0.0;
	x[1] = 1.0;
	x[2] = 2.0;
	run_test<Powell3D, 3>(x);

	xvec[0] = x[0];
	xvec[1] = x[1];
	xvec[2] = x[2];
	f.evaluate(xvec, &g);
	auto tmpstr = to_string("x = (", std::setprecision(16), x[0], ", ", x[1], ", ", x[2], ")\n");
	INFO(tmpstr);
	INFO("g = (" << g.transpose() << ")");

	// The webpage states that the optimal point is
	// (1, 1, 1), but that seems incorrect.
	//EXPECT_LT( std::fabs(x[0] - 1.0), 1e-8);
	//EXPECT_LT( std::fabs(x[1] - 1.0), 1e-8);
	//EXPECT_LT( std::fabs(x[2] - 1.0), 1e-8);
}

// #43
struct Himmelblau
{
	template<typename R>
	R operator()(const R* const x) const
	{
		// f = ( x(1)^2 + x(2) - 11.0 )^2 + ( x(1) + x(2)^2 - 7.0 )^2;
		R d1 = x[0]*x[0] + x[1]      - 11.0;
		R d2 = x[0]      + x[1]*x[1] - 7.0;
		return d1*d1 + d2*d2;
	}
};

TEST(Solver, Himmelblau)
{
	double x[2] = {-1.3, 2.7};
	double fval = run_test<Himmelblau, 2>(x);

	EXPECT_LT( std::fabs(fval), 1e-8);
}

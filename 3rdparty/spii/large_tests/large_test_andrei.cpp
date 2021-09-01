// Petter Strandmark 2013.
//
// Test functions from
// Neculai Andrei, An Unconstrained Optimization Test Functions Collection,
// Advanced Modeling and Optimization, Volume 10, Number 1, 2008.
//

#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <spii/google_test_compatibility.h>

// Defines run_test.
#include "large_test_header.h"

const bool all_methods = true;
const bool no_newton = false;

#define BEGIN_MODEL1(Model) \
	struct Model \
	{ \
		template<typename R>  \
		R operator()(const R* const x) const \
		{ \
			R value = 0;

#define BEGIN_MODEL2(Model) \
	struct Model \
	{ \
		template<typename R>  \
		R operator()(const R* const x, const R* const y) const \
		{ \
			R value = 0;

#define END_MODEL \
			return value; \
		} \
	}; 

#define LARGE_SUITE_BEGIN(Model) \
	TEST_CASE(#Model, "") \
	{ \
		auto create_function = [](std::vector<double>& start, Function* f) -> void \
		{ \
			auto n = start.size();
#define LARGE_SUITE_MIDDLE \
		}; \
		auto start_value = [](int n) -> std::vector<double> \
		{ \
			std::vector<double> value(n);
#define LARGE_SUITE_END(test_newton) \
			return value; \
		}; \
		run_test(create_function, start_value, test_newton); \
	}

BEGIN_MODEL2(FLETCBV3a)
	value = 0.5 * (x[0]*x[0] + y[0]*y[0]);
END_MODEL

BEGIN_MODEL2(FLETCBV3b)
	R d = x[0] - y[0];
	value = 0.5 * d*d;
END_MODEL

struct FLETCBV3c 
{ 
	double h;
	FLETCBV3c(double _h) : h(_h) { }
	template<typename R> 
	R operator()(const R* const x) const
	{
		R value = 0;
		value -= (h*h + 2.0) / (h*h) * x[0] + 1.0 / (h*h) * cos(x[0]);
END_MODEL

LARGE_SUITE_BEGIN(FLETCBV3)
	for (int i = 0; i < n; ++i) {
		f->add_variable(&start[i], 1);
	}
	f->add_term(std::make_shared<AutoDiffTerm<FLETCBV3a, 1, 1>>(), &start[0], &start[n-1]);

	for (int i = 0; i < n-1; ++i) {
		f->add_term(std::make_shared<AutoDiffTerm<FLETCBV3b, 1, 1>>(), &start[i], &start[i+1]);
	}

	for (int i = 0; i < n; ++i) {
		f->add_term(std::make_shared<AutoDiffTerm<FLETCBV3c, 1>>(1.0), &start[i]);
	}
LARGE_SUITE_MIDDLE
	const double h = 1.0 / (n + 1.0);
	for (int i = 0; i < n; ++i) {
		value[i] = (i+1) * h;
	}
LARGE_SUITE_END(all_methods)


struct DIXMAANa
{
	double C;
	DIXMAANa(double alpha, int i, int n, double k1)
	{
		C = alpha * std::pow(double(i) / double(n), k1);
	}

	template<typename R> 
	R operator()(const R* const x) const
	{
		return C * x[0]*x[0];
	}
};

struct DIXMAANb
{
	double C;
	DIXMAANb(double beta, int i, int n, double k2)
	{
		C = beta * std::pow(double(i) / double(n), k2);
	}

	template<typename R> 
	R operator()(const R* const xi, const R* const xip1) const
	{
		auto tmp = xip1[0] + xip1[0]*xip1[0];
		return C * xi[0]*xi[0] * tmp*tmp;
	}
};

struct DIXMAANc
{
	double C;
	DIXMAANc(double gamma, int i, int n, double k3)
	{
		C = gamma * std::pow(double(i) / double(n), k3);
	}

	template<typename R> 
	R operator()(const R* const xi, const R* const xipm) const
	{
		return C * xi[0]*xi[0] * xipm[0]*xipm[0]*xipm[0]*xipm[0];
	}
};

struct DIXMAANd
{
	double C;
	DIXMAANd(double delta, int i, int n, double k4)
	{
		C = delta * std::pow(double(i) / double(n), k4);
	}

	template<typename R> 
	R operator()(const R* const xi, const R* const xip2m) const
	{
		return C * xi[0] * xip2m[0];
	}
};

void run_DIXMAAN_test(double alpha, double beta, double gamma, double delta,
                      int k1, int k2, int k3, int k4)
{
	CAPTURE(alpha);
	CAPTURE(beta);
	CAPTURE(gamma);
	CAPTURE(delta);
	CAPTURE(k1);
	CAPTURE(k2);
	CAPTURE(k3);
	CAPTURE(k4);

	auto create_function = [&](std::vector<double>& start, Function* f) -> void
	{
		using std::make_shared;
		auto n = int(start.size());
		auto m = n / 3;

		for (int i = 1; i <= n; ++i) {
			f->add_term(make_shared<AutoDiffTerm<DIXMAANa, 1>>(alpha, i, n, k1), &start[i-1]);
		}

		for (int i = 1; i <= n - 1; ++i) {
			f->add_term(make_shared<AutoDiffTerm<DIXMAANb, 1, 1>>(alpha, i, n, k1), &start[i-1], &start[i]);
		}

		for (int i = 1; i <= 2*m; ++i) {
			f->add_term(make_shared<AutoDiffTerm<DIXMAANc, 1, 1>>(alpha, i, n, k1), &start[i-1], &start[i+m-1]);
		}

		for (int i = 1; i <= m; ++i) {
			f->add_term(make_shared<AutoDiffTerm<DIXMAANd, 1, 1>>(alpha, i, n, k1), &start[i-1], &start[i+2*m-1]);
		}
	};

	auto start_value = [](int n) -> std::vector<double>
	{
		std::vector<double> value(n, 2.0);
		return value;
	};

	run_test(create_function, start_value, all_methods);
}

TEST_CASE("DIXMAANA")
{
	run_DIXMAAN_test(1.0, 0.0, 0.125, 0.125, 0, 0, 0, 0);
}

TEST_CASE("DIXMAANB")
{
	run_DIXMAAN_test(1.0, 0.0625, 0.0625, 0.0625, 0, 0, 0, 1);
}

TEST_CASE("DIXMAANC")
{
	run_DIXMAAN_test(1.0, 0.125, 0.125, 0.125, 0, 0, 0, 0);
}

TEST_CASE("DIXMAAND")
{
	run_DIXMAAN_test(1.0, 0.26, 0.26, 0.26, 0, 0, 0, 0);
}

TEST_CASE("DIXMAANE")
{
	run_DIXMAAN_test(1.0, 0.0, 0.125, 0.125, 1, 0, 0, 1);
}

TEST_CASE("DIXMAANF")
{
	run_DIXMAAN_test(1.0, 0.0625, 0.0625, 0.0625, 1, 0, 0, 1);
}

TEST_CASE("DIXMAANG")
{
	run_DIXMAAN_test(1.0, 0.125, 0.125, 0.125, 1, 0, 0, 1);
}

TEST_CASE("DIXMAANH")
{
	run_DIXMAAN_test(1.0, 0.26, 0.26, 0.26, 1, 0, 0, 1);
}

TEST_CASE("DIXMAANI")
{
	run_DIXMAAN_test(1.0, 0.0, 0.125, 0.125, 1, 0, 0, 1);
}

TEST_CASE("DIXMAANJ")
{
	run_DIXMAAN_test(1.0, 0.0625, 0.0625, 0.0625, 1, 0, 0, 1);
}

TEST_CASE("DIXMAANK")
{
	run_DIXMAAN_test(1.0, 0.125, 0.125, 0.125, 1, 0, 0, 1);
}

TEST_CASE("DIXMAANL")
{
	run_DIXMAAN_test(1.0, 0.26, 0.26, 0.26, 1, 0, 0, 1);
}

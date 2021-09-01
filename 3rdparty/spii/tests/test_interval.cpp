// Petter Strandmark 2013–2014.

#include <iostream>

#include <catch.hpp>

#include <spii/interval.h>
using namespace spii;

namespace
{
void check_approximately_equal(Interval<double> i1, Interval<double> i2)
{
	CHECK(Approx(i1.get_lower()) == i2.get_lower()); 
	CHECK(Approx(i1.get_upper()) == i2.get_upper());
}
}

TEST_CASE("get")
{
	Interval<double> i1(123, 456);

	CHECK(i1.get_lower() == 123);
	CHECK(i1.get_upper() == 456);
}

TEST_CASE("length")
{
	Interval<int> i1(123, 456);

	CHECK(i1.length() == 456 - 123);
}

TEST_CASE("invalid_construction")
{
	CHECK_THROWS(Interval<double>(2, 1));
	CHECK_NOTHROW(Interval<int>(1, 1));
}

TEST_CASE("plus")
{
	Interval<double> i1(1, 2);
	Interval<double> i2(3, 4);

	INFO(i1 << " + " << 10 << " = " << i1 + 10.0);
	CHECK((i1 + 10.0) == Interval<double>(11, 12));
	CHECK((10.0 + i1) == Interval<double>(11, 12));
	CHECK((i1 + i2) == Interval<double>(4, 6));
}

TEST_CASE("plus-double-int")
{
	Interval<double> i1(1, 2);
	INFO(i1 << " + " << 10 << " = " << i1 + 10.0);
	CHECK((i1 + 10) == Interval<double>(11, 12));
	CHECK((10 + i1) == Interval<double>(11, 12));
}

TEST_CASE("minus")
{
	Interval<double> i1(1, 2);
	Interval<double> i2(3, 4);

	CHECK((i1 - 10.0) == Interval<double>(-9, -8));
	CHECK((10.0 - i1) == Interval<double>(8, 9));
	CHECK((i1 - i2) == Interval<double>(-3, -1));
	CHECK((i1 - i2) == -(i2 - i1));
}

TEST_CASE("minus-double-int")
{
	Interval<double> i1(1, 2);
	CHECK((i1 - 10) == Interval<double>(-9, -8));
	CHECK((10 - i1) == Interval<double>(8, 9));
}

TEST_CASE("multiplication")
{
	Interval<double> i1(1, 2);

	CHECK((i1 *  5.0) == Interval<double>(5, 10));
	CHECK((5.0  * i1) == Interval<double>(5, 10));
	CHECK((i1 * -5.0) == Interval<double>(-10, -5));
	CHECK((-5.0 * i1) == Interval<double>(-10, -5));

	Interval<double> i2(-1, 2);

	CHECK((i2 *  5.0) == Interval<double>(-5, 10));
	CHECK((5.0  * i2) == Interval<double>(-5, 10));
	CHECK( (i2 * -5.0) == Interval<double>(-10, 5));
	CHECK((-5.0 * i2) == Interval<double>(-10, 5));

	CHECK((i1 * i2) == Interval<double>(-2, 4));
}

TEST_CASE("multiplication-double-int")
{
	Interval<double> i1(1, 2);

	CHECK((i1 *  5) == Interval<double>(5, 10));
	CHECK((5  * i1) == Interval<double>(5, 10));
	CHECK((i1 * -5) == Interval<double>(-10, -5));
	CHECK((-5 * i1) == Interval<double>(-10, -5));

	Interval<double> i2(-1, 2);

	CHECK((i2 *  5) == Interval<double>(-5, 10));
	CHECK((5  * i2) == Interval<double>(-5, 10));
	CHECK((i2 * -5) == Interval<double>(-10, 5));
	CHECK((-5 * i2) == Interval<double>(-10, 5));
}

TEST_CASE("multiplication_all_sign_combinations")
{
	int lim = 3;
	for (int i1 = -lim; i1 <= +lim; ++i1) {
	for (int i2 = i1 + 1; i2 <= +lim; ++i2) {
		for (int j1 = -lim; j1 <= +lim; ++j1) {
		for (int j2 = j1 +1; j2 <= +lim; ++j2) {
			Interval<int> interval1(i1, i2);
			Interval<int> interval2(j1, j2);
			CHECK((interval1 * interval2) == (interval2 * interval1));
		}}
	}}
}

TEST_CASE("division")
{
	Interval<double> result;

	Interval<double> i1(1.0, 2.0);
	result = i1 / 3.0;
	CHECK(result.get_lower() == 1.0 / 3.0);
	CHECK(result.get_upper() == 2.0 / 3.0);

	Interval<double> i2(-1.0, 2.0);
	result = i2 / 3.0;
	CHECK(result.get_lower() == - 1.0 / 3.0);
	CHECK(result.get_upper() ==   2.0 / 3.0);

	Interval<double> i3(1.0, 2.0);
	result = i3 / -3.0;
	CHECK(result.get_lower() == - 2.0 / 3.0);
	CHECK(result.get_upper() == - 1.0 / 3.0);

	Interval<double> i4(1.0, 2.0);
	result = 3.0 / i4;
	CHECK(result.get_lower() == 3.0 / 2.0);
	CHECK(result.get_upper() == 3.0);

	Interval<double> i5(-1.0, 1.0);
	result = 3.0 / i5;
	CHECK(result.get_lower() == - std::numeric_limits<double>::infinity());
	CHECK(result.get_upper() ==   std::numeric_limits<double>::infinity());
}

TEST_CASE("division-double-int")
{
	Interval<double> result;

	Interval<double> i1(1.0, 2.0);
	result = i1 / 3;
	CHECK(result.get_lower() == 1.0 / 3.0);
	CHECK(result.get_upper() == 2.0 / 3.0);

	Interval<double> i2(-1.0, 2.0);
	result = i2 / 3;
	CHECK(result.get_lower() == -1.0 / 3.0);
	CHECK(result.get_upper() == 2.0 / 3.0);

	Interval<double> i3(1.0, 2.0);
	result = i3 / -3;
	CHECK(result.get_lower() == -2.0 / 3.0);
	CHECK(result.get_upper() == -1.0 / 3.0);

	Interval<double> i4(1.0, 2.0);
	result = 3 / i4;
	CHECK(result.get_lower() == 3.0 / 2.0);
	CHECK(result.get_upper() == 3.0);

	Interval<double> i5(-1.0, 1.0);
	result = 3 / i5;
	CHECK(result.get_lower() == -std::numeric_limits<double>::infinity());
	CHECK(result.get_upper() == std::numeric_limits<double>::infinity());
}

TEST_CASE("division_strictly_positive")
{
	int lim = 4;
	for (int i1 = 1; i1 <= +lim; ++i1) {
	for (int i2 = i1 + 1; i2 <= +lim; ++i2) {
		for (int j1 = 1; j1 <= +lim; ++j1) {
		for (int j2 = j1 +1; j2 <= +lim; ++j2) {
			if (i1 != 0 && i2 != 0) {
				Interval<double> interval1(i1, i2);
				Interval<double> interval2(j1, j2);
				CAPTURE(interval1);
				CAPTURE(interval2);
				CHECK((interval1 / interval2).get_lower() > 0);
			}
		}}
	}}
}

TEST_CASE("division_as_multiplication")
{
	int lim = 4;
	for (int i1 = -lim; i1 <= +lim; ++i1) {
	for (int i2 = i1 + 1; i2 <= +lim; ++i2) {
		for (int j1 = 1; j1 <= +lim; ++j1) {
		for (int j2 = j1 +1; j2 <= +lim; ++j2) {
			if (i1 != 0 && i2 != 0) {
				Interval<double> interval1(i1, i2);
				Interval<double> interval2(j1, j2);
				CAPTURE(interval1);
				CAPTURE(interval2);
				auto inv2 = 1.0 / interval2;
				CHECK((interval1 / interval2) == (inv2 * interval1));
			}
		}}
	}}
}

TEST_CASE("cos")
{
	Interval<double> result;

	Interval<double> i1(2.0, 6.0);
	result = cos(i1);
	CHECK(result.get_lower() == -1.0);
	CHECK(Approx(result.get_upper()) == cos(6.0));

	Interval<double> i2(-10.0, -4.0);
	result = cos(i2);
	CHECK(result.get_lower() == -1.0);
	CHECK(result.get_upper() == +1.0);

	Interval<double> i3(-10.0, -8.0);
	result = cos(i3);
	CHECK(result.get_lower() == -1.0);
	CHECK(Approx(result.get_upper()) == cos(8.0));

	Interval<double> i4(4.0, 8.0);
	result = cos(i4);
	CHECK(Approx(result.get_lower()) == cos(4.0));
	CHECK(result.get_upper() == +1.0);

	Interval<double> i5(2.0, 2.5);
	result = cos(i5);
	CHECK(Approx(result.get_lower()) == cos(2.5));
	CHECK(Approx(result.get_upper()) == cos(2.0));
}

TEST_CASE("sin")
{
	using namespace std;

	Interval<double> i1(-10.0, 10.0);
	CHECK(sin(i1) == Interval<double>(-1.0, 1.0));
	Interval<double> i2(3.0, 3.0);
	CHECK(Approx(sin(i2).get_lower()) == sin(3.0));
	CHECK(Approx(sin(i2).get_upper()) == sin(3.0));
}

TEST_CASE("abs")
{
	Interval<double> i1(-10.0, 10.0);
	CHECK(abs(sin(i1)) == Interval<double>(0.0, 1.0));
}

TEST_CASE("odd_powers")
{
	using std::pow;
	Interval<double> i(-2.0, 3.0);
	CHECK(pow(i, 3) == Interval<double>(-8, 27));
}

TEST_CASE("even_powers")
{
	using std::pow;
	Interval<double> i1(2.0, 3.0);
	CHECK(pow(i1, 2) == Interval<double>(4, 9));

	Interval<double> i2(-3.0, -2.0);
	CHECK(pow(i2, 2) == Interval<double>(4, 9));

	Interval<double> i3(-2.0, 3.0);
	CHECK(pow(i3, 2) == Interval<double>(0, 9));
}

TEST_CASE("double_exponents")
{
	using std::pow;
	Interval<double> i1(2.0, 3.0);
	check_approximately_equal(pow(i1, 2), pow(i1, 2.0));

	Interval<double> i2(1.0, 5.0);
	check_approximately_equal(pow(i2, 2), pow(i2, 2.0));

	Interval<double> i3(-2.0, 7.0);
	CHECK_THROWS(pow(i3, 2.0));
}

TEST_CASE("interval_exponents")
{
	using std::pow;
	Interval<double> i1(2.0, 3.0);
	Interval<double> i2(-3.0, -2.0);
	CHECK(pow(i1, i2).contains(pow(2.0, -2.5)));
	CHECK(pow(i1, i2).contains(pow(3.0, -2.5)));
	CHECK_THROWS(pow(i2, i2));
}

// Rump’s example.
// See section 1.4.1 in “Global Optimization Using Interval Analysis” by Eldon Hansen.
template<typename R>
R rump(R x, R y)
{
	using std::pow;
	return (333.75 - pow(x, 2)) * pow(y, 6)
		+ pow(x, 2) * (11.0 * pow(x, 2) * pow(y, 2) - 121.0 * pow(y, 4) - 2.0)
		+ 5.5 * pow(y, 8) + x / (2.0 * y);
}

TEST_CASE("Rump")
{
	double x0 = 77617;
	double y0 = 33096;
	double gt = -0.82739605994682136814116509;
	
	// Fails due to the difficulty of Rump’s function.
	//CHECK(Approx(rump(x0, y0)) == gt);

	double eps = 1e-14;
	Interval<double> ix(x0 - x0*eps, x0 + x0*eps);
	Interval<double> iy(y0 - y0*eps, y0 + y0*eps);
	CAPTURE(rump(ix, iy));  // Really wide!
	CHECK(rump(ix, iy).get_lower() <= gt);
	CHECK(rump(ix, iy).get_upper() >= gt);
}

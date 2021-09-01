// Petter Strandmark 2012.

#include <catch.hpp>

#include <spii/spii.h>

using namespace spii;
using namespace std;

TEST_CASE("spii_assert", "")
{
	CHECK_THROWS_AS(spii_assert(1 == 2, "message"), logic_error);
	CHECK_NOTHROW(spii_assert(1 == 1, "message"));

	try {
		spii_assert(1 == 2, "message");
	}
	catch (std::logic_error& err) {
		// Will print the stack trace to the test output.
		INFO(err.what());
		SUCCEED();
	}
}

TEST_CASE("check", "")
{
	CHECK_THROWS_AS(check(1 == 2, "1 is not 2"), runtime_error);
	CHECK_NOTHROW(check(1 == 1, "Something is very wrong."));
}

TEST_CASE("check-message", "")
{
	try {
		check(1 == 2, "1 ", "is ", "not ",  2);
	}
	catch (std::runtime_error& err) {
		CHECK(err.what() == std::string{"1 is not 2"});
	}
}

TEST_CASE("scope_exit")
{
	int i = 1;
	{
		spii_at_scope_exit( i = 3 );
	}
	CHECK(i == 3);

	int j = 1;
	{
		auto guard = spii::make_scope_guard([&](){ j = 3; });
		guard.dismiss();
	}
	CHECK(j == 1);

	i = 1;
	j = 1;
	{
		spii_at_scope_exit( i = 3 );
		spii_at_scope_exit( j = 3 );
		i = 1;
		j = 1;
	}
	CHECK(i == 3);
	CHECK(j == 3);
}

// Petter Strandmark 2013–2014.

#include <catch.hpp>

#include <spii/string_utils.h>

#include <list>
#include <map>

using namespace spii;
using namespace std;

TEST_CASE("to_string")
{
	CHECK(to_string("Test", 12, "test") == "Test12test");
}

TEST_CASE("to_string_pair")
{
	pair<int, string> p{123, "Test"};
	CHECK(to_string(p) == "(123, Test)");
}

TEST_CASE("to_string_tuple")
{
	auto t = make_tuple(123, "Test", 'A');
	CHECK(to_string(t) == "(123, Test, A)");
}

TEST_CASE("to_string_pair_pair")
{
	auto p  = make_pair(123, "Test");
	auto pp = make_pair(12.1, p);
	CHECK(to_string(pp) == "(12.1, (123, Test))");
}

TEST_CASE("to_string_vector")
{
	vector<int> v{1, 2, 3};
	CHECK(to_string(v) == "[1, 2, 3]");
	v.clear();
	CHECK(to_string(v) == "[]");
}

TEST_CASE("to_string_vector_pair")
{
	vector<pair<int, string>> v{{1, "P"}, {2, "S"}};
	CHECK(to_string(v) == "[(1, P), (2, S)]");
}

TEST_CASE("to_string_set")
{
	set<int> v{1, 2, 3};
	CHECK(to_string(v) == "{1, 2, 3}");
	v.clear();
	CHECK(to_string(v, v, v) == "{}{}{}");
}

TEST_CASE("to_string_map")
{
	map<int, string> m;
	m[1] = "P";
	m[0] = "E";
	m[2] = "T";
	CHECK(to_string(m) == "[(0, E), (1, P), (2, T)]");
	CHECK(to_string(m, " ", map<int, int>{}) == "[(0, E), (1, P), (2, T)] []");
	m.clear();
	CHECK(to_string(m) == "[]");
}

TEST_CASE("to_string_setprecision")
{
	double d = 1.123456;
	CHECK(to_string(setprecision(2), d) == "1.1");
}

TEST_CASE("to_string_setfill_setw")
{
	CHECK(to_string(setfill('P'), setw(3), 1) == "PP1");
}

TEST_CASE("format_string_1")
{
	CHECK(format_string("Test %0!", 12) == "Test 12!");
	CHECK(format_string("Test %0", 12) == "Test 12");
}

TEST_CASE("format_string_%")
{
	CHECK(format_string("Test %0%%", 12) == "Test 12%");
	CHECK(format_string("Test %0%%!", 12) == "Test 12%!");
	CHECK(format_string("Test %0 %%", 12) == "Test 12 %");
	CHECK(format_string("Test %0 %%!", 12) == "Test 12 %!");
}

TEST_CASE("format_string_2")
{
	CHECK(format_string("Test %0 and %1!", 'A', 'O') == "Test A and O!");
	CHECK(format_string("Test %1 and %0!", 'A', 'O') == "Test O and A!");
}

TEST_CASE("from_string")
{
	CHECK(from_string<int>("42") == 42);
	CHECK(from_string("asd", 42) == 42);
	CHECK_THROWS(from_string<int>("abc"));
}

TEST_CASE("join")
{
	std::vector<int> v1 = {1, 2, 3};
	CHECK(join('\t', v1) == "1\t2\t3");
	CHECK(join("x", v1) == "1x2x3");

	std::vector<int> v2 ;
	CHECK(join('\t', v2) == "");
	CHECK(join("\t", v2) == "");

	CHECK(join('\t', {1, 2, 3}) == "1\t2\t3");
	CHECK(join("\t", {1, 2, 3}) == "1\t2\t3");
}

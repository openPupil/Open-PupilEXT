// Petter Strandmark 2012.

#include <catch.hpp>

#include <spii/auto_diff_term.h>
#include <spii/term_factory.h>

using namespace spii;
using namespace std;

template<int dim>
struct Dummy
{
	template<typename R>
	R operator()(const R* const x) const
	{
		return 0.;
	}
};

TEST_CASE("TermFactory/simple_test", "")
{
	TermFactory factory;
	factory.teach_term<AutoDiffTerm<Dummy<1>,1>>();
	factory.teach_term<AutoDiffTerm<Dummy<2>,2>>();

	auto term1 = factory.create(typeid(AutoDiffTerm<Dummy<1>,1>).name(), cin);
	REQUIRE(term1->number_of_variables() == 1);
	REQUIRE(term1->variable_dimension(0) == 1);

	auto term2 = factory.create(typeid(AutoDiffTerm<Dummy<2>,2>).name(), cin);
	REQUIRE(term2->number_of_variables() == 1);
	REQUIRE(term2->variable_dimension(0) == 2);
}

TEST_CASE("TermFactory/throws_on_unknown", "")
{
	TermFactory factory;
	CHECK_THROWS_AS(factory.create(typeid(AutoDiffTerm<Dummy<1>,1>).name(), cin), logic_error);
}

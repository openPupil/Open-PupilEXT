// Petter Strandmark 2013.
#include <map>
#include <stdexcept>
#include <vector>

#include <spii/term_factory.h>

using namespace std;

namespace spii
{

class TermFactory::Implementation
{
public:
	map<string, TermCreator> creators;
};

TermFactory::TermFactory()
	: impl{new TermFactory::Implementation}
{
}

TermFactory::~TermFactory()
{
	delete impl;
}

std::shared_ptr<const Term> TermFactory::create(const std::string& term_name,
                                                std::istream& in) const
{
	auto creator = impl->creators.find(fix_name(term_name));
	spii_assert(creator != impl->creators.end(),
		"TermFactory::create: Unknown Term ",
		term_name);
	return std::shared_ptr<const Term>(creator->second(in));
}

void TermFactory::teach_term(const std::string& term_name, const TermCreator& creator)
{
	impl->creators[fix_name(term_name)] = creator;
}

std::string TermFactory::fix_name(std::string org_name)
{
	std::string name = org_name;
	std::replace(name.begin(), name.end(), ' ', '-');
	return name;
}

}

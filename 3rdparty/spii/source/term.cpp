
#include <spii/term.h>

namespace spii
{

// This function only needs to be implemented if interval arithmetic is
// desired.
Interval<double> Term::evaluate_interval(const Interval<double> * const * const variables) const
{
	spii_assert(false, "evaluate_interval: Not implemented.");
	// To avoid warning.
	return {0, 0};
};

void Term::read(std::istream& in)
{
}

void Term::write(std::ostream& out) const
{
}

std::ostream& operator << (std::ostream& out, const Term& term)
{
	term.write(out);
	return out;
}

std::istream& operator >> (std::istream& in, Term& term)
{
	term.read(in);
	return in;
}

}  // namespace spii.


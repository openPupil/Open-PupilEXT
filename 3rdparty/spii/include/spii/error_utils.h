// Petter Strandmark 2013–2014.
#ifndef SPII_ERROR_UTILS_H
#define SPII_ERROR_UTILS_H

#include <stdexcept>
#include <string>

#include <spii/spii.h>

namespace spii
{

//
// Enables expressions like:
//
//    check(a == 42, a, " is not 42.");
//
// Will throw if expression is false.
//
template<typename... Args>
void check(bool everything_OK, Args&&... args)
{
	if (!everything_OK) {
		throw std::runtime_error(to_string(std::forward<Args>(args)...));
	}
}

void SPII_API verbose_error_internal(const char* expr, const char* full_file_cstr, int line, const char* args);

template<typename... Args>
void verbose_error(const char* expr, const char* full_file_cstr, int line, Args&&... args)
{
	verbose_error_internal(expr, full_file_cstr, line, to_string(std::forward<Args>(args)...).c_str());
}

//#define ASSERT(expr, ...) (expr) ? ((void)0) : spii::verbose_error(#expr, __FILE__, __LINE__, spii::to_string(__VA_ARGS__))

}

#endif

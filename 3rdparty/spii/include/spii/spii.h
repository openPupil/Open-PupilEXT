// Petter Strandmark 2012–2013.
#ifndef SPII_H
#define SPII_H

#ifdef _WIN32
#	ifdef spii_EXPORTS
#		define SPII_API __declspec(dllexport)
#		define SPII_API_EXTERN_TEMPLATE
#	else
#		define SPII_API __declspec(dllimport)
#		define SPII_API_EXTERN_TEMPLATE extern
#	endif
#else
#	define SPII_API
#	define SPII_API_EXTERN_TEMPLATE
#endif // WIN32

#include <sstream>
#include <stdexcept>
#include <string>

#include <spii/string_utils.h>
#include <spii/error_utils.h>

namespace spii
{

double SPII_API wall_time();
double SPII_API cpu_time();

#define spii_assert(expr, ...) (expr) ? ((void)0) : spii::verbose_error(#expr, __FILE__, __LINE__, spii::to_string(__VA_ARGS__))

//
// spii_at_scope_exit( statement; ) executes statement at the end 
// of the current scope.
//
template <typename F>
class ScopeGuard
{
public:
    ScopeGuard(F&& f)
		: f(std::forward<F>(f))
	{}

	ScopeGuard(ScopeGuard&& guard)
		: f(std::move(guard.f)), active(guard.active)
	{
		guard.dismiss();
	}

    ~ScopeGuard()
	{
		if (active) { 
			f();
		}
	}

	ScopeGuard(const ScopeGuard&) = delete;
	ScopeGuard& operator = (const ScopeGuard&) = delete;

	void dismiss()
	{
		active = false;
	}

private:
    F f;
	bool active = true;
};

template <typename F>
ScopeGuard<F> make_scope_guard(F&& f) {
    return std::move(ScopeGuard<F>(std::forward<F>(f)));
};

#define SPII_JOIN_PP_SYMBOLS_HELPER(arg1, arg2) arg1 ## arg2
#define SPII_JOIN_PP_SYMBOLS(arg1, arg2) SPII_JOIN_PP_SYMBOLS_HELPER(arg1, arg2)
#define spii_at_scope_exit(code) \
    auto SPII_JOIN_PP_SYMBOLS(spii_scope_exit_guard_, __LINE__) = ::spii::make_scope_guard([&](){code;})

}

#endif

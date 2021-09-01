include(CheckCXXSourceCompiles)

check_cxx_source_compiles("int main() { auto lambda = [](auto x, auto y) { return x + y; };  }" USE_GENERIC_LAMBDAS)

if (USE_GENERIC_LAMBDAS)
	message("-- Generic lambdas (C++14) are supported by the compiler.")
	add_definitions(-DUSE_GENERIC_LAMBDAS)
else ()
	message("-- Generic lambdas (C++14) are not supported by the compiler.")
endif()

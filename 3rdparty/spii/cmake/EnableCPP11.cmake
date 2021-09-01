
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
	include(CheckCXXCompilerFlag)
	check_cxx_compiler_flag(-std=c++14 SUPPORTS_STD_CXX14)
	check_cxx_compiler_flag(-std=c++1y SUPPORTS_STD_CXX1Y)
	check_cxx_compiler_flag(-std=c++11 SUPPORTS_STD_CXX11)
	check_cxx_compiler_flag(-std=c++0x SUPPORTS_STD_CXX0X)

	# My installation of Emscripten does not work with C++14 for
	# all C++ standard headers. Disabling C++14 for Emscripten
	# for now.

	if(SUPPORTS_STD_CXX14 AND NOT EMSCRIPTEN)
		message("-- Enabling C++14 support with -std=c++14.")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
	elseif(SUPPORTS_STD_CXX1Y AND NOT EMSCRIPTEN)
		message("-- Enabling C++14 support with -std=c++1y.")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
	elseif(SUPPORTS_STD_CXX11)
		message("-- Enabling C++11 support with -std=c++11.")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	elseif(SUPPORTS_STD_CXX0X)
		message("-- Enabling C++11 support with -std=c++0x.")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	else()
		message(ERROR "Compiler does not support C++11.")
	endif()
endif()

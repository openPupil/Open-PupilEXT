# Author: petter.strandmark@gmail.com (Petter Strandmark)

add_library(Catch main_test_program.cpp)

macro (spii_test NAME)
	add_executable(${NAME}${EXECUTABLE_EXTENSION}
	               ${NAME}.cpp
	               ${SPII_HEADERS})

	target_link_libraries(${NAME}${EXECUTABLE_EXTENSION} spii Catch)
	set_property(TARGET ${NAME}${EXECUTABLE_EXTENSION}         PROPERTY FOLDER "Tests")
	add_test(NAME ${NAME}${EXECUTABLE_EXTENSION}
	         COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
endmacro ()

file(GLOB TEST_FILES test_*.cpp)
file(GLOB MESCHACH_TEST test_meschach*.cpp)
list(REMOVE_ITEM TEST_FILES ${MESCHACH_TEST})
file(GLOB STRING_TEST test_string*.cpp)
list(REMOVE_ITEM TEST_FILES ${STRING_TEST})
list(REMOVE_ITEM TEST_FILES test_nist_global.cpp)

foreach (TEST_FILE ${TEST_FILES})
	get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
	message("-- Adding test: " ${TEST_NAME})
	spii_test(${TEST_NAME})
endforeach()

# Meschach test does not link to spii.
add_executable(test_meschach${EXECUTABLE_EXTENSION} test_meschach.cpp)
	target_link_libraries(test_meschach${EXECUTABLE_EXTENSION} meschach Catch)
	add_test(NAME meschach${EXECUTABLE_EXTENSION}
	       COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_meschach${EXECUTABLE_EXTENSION})
	set_property(TARGET test_meschach${EXECUTABLE_EXTENSION}         PROPERTY FOLDER "Tests")

# String test does not link to spii.
add_executable(test_string${EXECUTABLE_EXTENSION} test_string.cpp)
	target_link_libraries(test_string${EXECUTABLE_EXTENSION} Catch)
	add_test(NAME string${EXECUTABLE_EXTENSION}
	       COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_string${EXECUTABLE_EXTENSION})
	set_property(TARGET test_string${EXECUTABLE_EXTENSION}         PROPERTY FOLDER "Tests")

#
# Valgrind (memcheck) testing.
#
find_program(VALGRIND NAMES valgrind PATH /usr/bin /usr/local/bin)
if (VALGRIND)
	message("-- Valgrind found; configuring memcheck tests.")
	macro(valgrind_test NAME)
		add_test(NAME memcheck_${NAME}
		         COMMAND ${VALGRIND} --error-exitcode=1 --leak-check=full ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_${NAME})
	endmacro()

	# Run shorter tests though memcheck.
	valgrind_test(function)
	valgrind_test(function_serializer)
	valgrind_test(solver)
	valgrind_test(spii)
	valgrind_test(term)
	valgrind_test(term_factory)
endif ()

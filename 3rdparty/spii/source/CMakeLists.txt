# Author: petter.strandmark@gmail.com (Petter Strandmark)

file(GLOB SPII_INTERNAL_SRC *.cpp)

set(SPII_LIBRARY_SOURCE
	${SPII_INTERNAL_SRC}
	${SPII_HEADERS})

add_library(spii SHARED ${SPII_LIBRARY_SOURCE})
target_link_libraries(spii ${SPII_LIBRARY_DEPENDENCIES})
target_link_libraries(spii meschach)

install(TARGETS spii
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

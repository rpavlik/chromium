set(SOURCES
	packperf.c
)
add_executable(packperf ${SOURCES})
target_link_libraries(packperf crpacker crutil)

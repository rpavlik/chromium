include_directories(.)
set(SOURCES
	packperf.c
)
add_executable(packperf  ${SOURCES})
target_link_libraries(packperf crpacker crutil)
target_link_libraries(packperf ${EXTRA_LIBS})
install(TARGETS packperf
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

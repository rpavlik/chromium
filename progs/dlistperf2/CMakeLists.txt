include_directories(.)
set(SOURCES
	dlistperf2.c
	OGLwin.c
	Timer.c
	global.c
)
add_executable(dlistperf2  ${SOURCES})
target_link_libraries(dlistperf2 ${EXTRA_LIBS})
install(TARGETS dlistperf2
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

include_directories(.)
set(SOURCES
	dlistperf1.c
	OGLwin.c
	Timer.c
	global.c
)
add_executable(dlistperf1  ${SOURCES})
target_link_libraries(dlistperf1 ${EXTRA_LIBS})

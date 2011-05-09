include_directories(.)
set(SOURCES
	dlistperf2.c
	OGLwin.c
	Timer.c
	global.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(dlistperf2  ${SOURCES})
target_link_libraries(dlistperf2 ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(dlistperf2 ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(dlistperf2 ${X11_LIBRARIES})
endif()
install(TARGETS dlistperf2
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

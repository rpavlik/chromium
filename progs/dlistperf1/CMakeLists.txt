include_directories(.)
set(SOURCES
	dlistperf1.c
	OGLwin.c
	Timer.c
	global.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(dlistperf1  ${SOURCES})
target_link_libraries(dlistperf1 ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(dlistperf1 ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(dlistperf1 ${X11_LIBRARIES})
endif()
install(TARGETS dlistperf1
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

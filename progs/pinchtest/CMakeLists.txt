include_directories(.)
set(SOURCES
	pinchtest.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(pinchtest  ${SOURCES})
target_link_libraries(pinchtest ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(pinchtest ${EXTRA_LIBS})
install(TARGETS pinchtest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

include_directories(.)
set(SOURCES
	bboxtest.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(bboxtest  ${SOURCES})
target_link_libraries(bboxtest ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(bboxtest ${EXTRA_LIBS})
install(TARGETS bboxtest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

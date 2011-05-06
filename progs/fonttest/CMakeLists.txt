include_directories(.)
set(SOURCES
	fonttest.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(fonttest  ${SOURCES})
target_link_libraries(fonttest ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(fonttest ${EXTRA_LIBS})
install(TARGETS fonttest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

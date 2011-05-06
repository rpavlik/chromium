include_directories(.)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

set(SOURCES
	main.c
)
add_executable(crutserver  ${SOURCES})
target_link_libraries(crutserver crmothership crutil crutapi)
target_link_libraries(crutserver ${EXTRA_LIBS})
target_link_libraries(crutserver ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
install(TARGETS crutserver
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

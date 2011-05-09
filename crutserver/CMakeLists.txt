include_directories(.)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

set(SOURCES
	main.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(crutserver  ${SOURCES})
target_link_libraries(crutserver crmothership crutil crutapi ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(crutserver ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(crutserver ${X11_LIBRARIES})
endif()
target_link_libraries(crutserver ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
install(TARGETS crutserver
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

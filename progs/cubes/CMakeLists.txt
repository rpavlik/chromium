include_directories(.)
set(SOURCES
	cubes.c
	timer.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(cubes  ${SOURCES})
target_link_libraries(cubes ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(cubes ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(cubes ${X11_LIBRARIES})
endif()
install(TARGETS cubes
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

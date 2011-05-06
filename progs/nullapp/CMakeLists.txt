include_directories(.)
set(SOURCES
	nullapp.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(nullapp  ${SOURCES})
target_link_libraries(nullapp ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(nullapp ${EXTRA_LIBS})
install(TARGETS nullapp
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

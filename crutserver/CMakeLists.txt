include_directories(.)
set(SOURCES
	main.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(crutserver  ${SOURCES})
target_link_libraries(crutserver crmothership crutil crutapi ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(crutserver ${EXTRA_LIBS})
install(TARGETS crutserver
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

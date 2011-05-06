include_directories(.)
set(SOURCES
	main.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(EXT_secondary_color  ${SOURCES})
target_link_libraries(EXT_secondary_color crutil crextensionscommon ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(EXT_secondary_color ${EXTRA_LIBS})
install(TARGETS EXT_secondary_color
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

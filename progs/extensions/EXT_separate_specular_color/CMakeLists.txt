include_directories(.)
set(SOURCES
	main.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(EXT_separate_specular_color  ${SOURCES})
target_link_libraries(EXT_separate_specular_color crutil crextensionscommon ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(EXT_separate_specular_color ${EXTRA_LIBS})
install(TARGETS EXT_separate_specular_color
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

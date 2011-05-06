include_directories(.)
set(SOURCES
	main.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(ARB_multitexture  ${SOURCES})
target_link_libraries(ARB_multitexture crutil crextensionscommon ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(ARB_multitexture ${EXTRA_LIBS})
install(TARGETS ARB_multitexture
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

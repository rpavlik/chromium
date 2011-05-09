include_directories(.)
set(SOURCES
	main.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(ARB_multitexture  ${SOURCES})
target_link_libraries(ARB_multitexture crutil crextensionscommon ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(ARB_multitexture ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(ARB_multitexture ${X11_LIBRARIES})
endif()
install(TARGETS ARB_multitexture
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

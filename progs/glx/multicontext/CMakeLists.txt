include_directories(.)
set(SOURCES
	multicontext.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(multicontext  ${SOURCES})
target_link_libraries(multicontext crutil crextensionscommon ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(multicontext ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(multicontext ${X11_LIBRARIES})
endif()
install(TARGETS multicontext
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

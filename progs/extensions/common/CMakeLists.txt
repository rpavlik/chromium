include_directories(.)
set(SOURCES
	logo.c
	util.c
	common.def
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_library(crextensionscommon SHARED ${SOURCES})
target_link_libraries(crextensionscommon crutil ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(crextensionscommon ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(crextensionscommon ${X11_LIBRARIES})
endif()
install(TARGETS crextensionscommon
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

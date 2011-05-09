include_directories(.)
set(SOURCES
	main.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(EXT_texture_edge_clamp  ${SOURCES})
target_link_libraries(EXT_texture_edge_clamp crutil crextensionscommon ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(EXT_texture_edge_clamp ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(EXT_texture_edge_clamp ${X11_LIBRARIES})
endif()
install(TARGETS EXT_texture_edge_clamp
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

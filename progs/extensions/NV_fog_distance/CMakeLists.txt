include_directories(.)
set(SOURCES
	main.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(NV_fog_distance  ${SOURCES})
target_link_libraries(NV_fog_distance crutil crextensionscommon ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(NV_fog_distance ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(NV_fog_distance ${X11_LIBRARIES})
endif()
install(TARGETS NV_fog_distance
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

include_directories(.)
set(SOURCES
	viewports.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(viewports  ${SOURCES})
target_link_libraries(viewports ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(viewports ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(viewports ${X11_LIBRARIES})
endif()
install(TARGETS viewports
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

include_directories(.)
set(SOURCES
	tiletest.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(tiletest  ${SOURCES})
target_link_libraries(tiletest ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(tiletest ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(tiletest ${X11_LIBRARIES})
endif()
install(TARGETS tiletest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

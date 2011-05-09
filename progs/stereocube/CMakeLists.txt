include_directories(.)
set(SOURCES
	stereocube.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(stereocube  ${SOURCES})
target_link_libraries(stereocube ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(stereocube ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(stereocube ${X11_LIBRARIES})
endif()
install(TARGETS stereocube
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

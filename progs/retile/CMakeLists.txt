include_directories(.)
set(SOURCES
	retile.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(retile  ${SOURCES})
target_link_libraries(retile ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(retile ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(retile ${X11_LIBRARIES})
endif()
install(TARGETS retile
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

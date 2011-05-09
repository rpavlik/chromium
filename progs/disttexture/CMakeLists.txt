include_directories(.)
set(SOURCES
	disttexture.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(disttexture  ${SOURCES})
target_link_libraries(disttexture ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(disttexture ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(disttexture ${X11_LIBRARIES})
endif()
install(TARGETS disttexture
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

include_directories(.)
set(SOURCES
	atlantis.c
	dolphin.c
	shark.c
	swim.c
	whale.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(atlantis  ${SOURCES})
target_link_libraries(atlantis ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(atlantis ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(atlantis ${X11_LIBRARIES})
endif()
install(TARGETS atlantis
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

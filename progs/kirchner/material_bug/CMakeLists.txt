include_directories(.)
set(SOURCES
	bug.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(kirchner_material  ${SOURCES})
target_link_libraries(kirchner_material ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(kirchner_material ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(kirchner_material ${X11_LIBRARIES})
endif()
install(TARGETS kirchner_material
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

include_directories(.)
set(SOURCES
	bug.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(kirchner_material  ${SOURCES})
target_link_libraries(kirchner_material ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(kirchner_material ${EXTRA_LIBS})
install(TARGETS kirchner_material
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

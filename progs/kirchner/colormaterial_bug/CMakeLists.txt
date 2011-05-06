include_directories(.)
set(SOURCES
	bug.c
)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(kirchner_colormaterial  ${SOURCES})
target_link_libraries(kirchner_colormaterial ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(kirchner_colormaterial ${EXTRA_LIBS})
install(TARGETS kirchner_colormaterial
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

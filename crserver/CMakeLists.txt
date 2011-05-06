
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})

include_directories(.)
set(SOURCES
	main.c
)
add_executable(crserver  ${SOURCES})
target_link_libraries(crserver crserverlib spuload crmothership crstate crpacker crutil crserver_crunpacker_copy)
target_link_libraries(crserver ${EXTRA_LIBS})
target_link_libraries(crserver ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
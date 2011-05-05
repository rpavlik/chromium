
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})

set(SOURCES
	main.c
)
add_executable(crserver ${SOURCES})
target_link_libraries(crserver crserverlib spuload crmothership crstate crpacker crutil)

target_link_libraries(crserver ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

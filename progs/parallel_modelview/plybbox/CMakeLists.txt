set(SOURCES
	../plyfile.c
	bbox.c
)
add_executable(plybbox ${SOURCES})
target_link_libraries(plybbox crutil)

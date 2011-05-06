include_directories(.)
set(SOURCES
	../plyfile.c
	bbox.c
)
add_executable(plybbox  ${SOURCES})
target_link_libraries(plybbox crutil)
target_link_libraries(plybbox ${EXTRA_LIBS})

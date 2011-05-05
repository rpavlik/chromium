set(SOURCES
	rawdump.c
)
add_executable(rawdump ${SOURCES})
target_link_libraries(rawdump crutil)

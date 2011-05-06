include_directories(.)
set(SOURCES
	rawdump.c
)
add_executable(rawdump  ${SOURCES})
target_link_libraries(rawdump crutil)
target_link_libraries(rawdump ${EXTRA_LIBS})

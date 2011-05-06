include_directories(.)
set(SOURCES
	cubes.c
	timer.c
)
add_executable(cubes  ${SOURCES})
target_link_libraries(cubes ${EXTRA_LIBS})

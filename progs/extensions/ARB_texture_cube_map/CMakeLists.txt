set(SOURCES
	main.c
)
add_executable(ARB_texture_cube_map ${SOURCES})
target_link_libraries(ARB_texture_cube_map crutil crextensionscommon)

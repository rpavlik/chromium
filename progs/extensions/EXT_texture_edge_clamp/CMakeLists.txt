set(SOURCES
	main.c
)
add_executable(EXT_texture_edge_clamp ${SOURCES})
target_link_libraries(EXT_texture_edge_clamp crutil crextensionscommon)

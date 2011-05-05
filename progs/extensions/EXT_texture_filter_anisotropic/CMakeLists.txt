set(SOURCES
	main.c
)
add_executable(EXT_texture_filter_anisotropic ${SOURCES})
target_link_libraries(EXT_texture_filter_anisotropic crutil crextensionscommon)

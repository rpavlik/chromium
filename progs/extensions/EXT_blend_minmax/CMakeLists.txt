set(SOURCES
	main.c
)
add_executable(EXT_blend_minmax ${SOURCES})
target_link_libraries(EXT_blend_minmax crutil crextensionscommon)

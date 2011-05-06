include_directories(.)
set(SOURCES
	main.c
)
add_executable(EXT_blend_color  ${SOURCES})
target_link_libraries(EXT_blend_color crutil crextensionscommon)
target_link_libraries(EXT_blend_color ${EXTRA_LIBS})

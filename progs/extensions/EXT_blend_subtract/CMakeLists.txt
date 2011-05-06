include_directories(.)
set(SOURCES
	main.c
)
add_executable(EXT_blend_subtract  ${SOURCES})
target_link_libraries(EXT_blend_subtract crutil crextensionscommon)
target_link_libraries(EXT_blend_subtract ${EXTRA_LIBS})

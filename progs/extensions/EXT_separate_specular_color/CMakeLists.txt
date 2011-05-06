include_directories(.)
set(SOURCES
	main.c
)
add_executable(EXT_separate_specular_color  ${SOURCES})
target_link_libraries(EXT_separate_specular_color crutil crextensionscommon)
target_link_libraries(EXT_separate_specular_color ${EXTRA_LIBS})

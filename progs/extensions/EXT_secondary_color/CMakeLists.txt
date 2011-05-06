include_directories(.)
set(SOURCES
	main.c
)
add_executable(EXT_secondary_color  ${SOURCES})
target_link_libraries(EXT_secondary_color crutil crextensionscommon)
target_link_libraries(EXT_secondary_color ${EXTRA_LIBS})

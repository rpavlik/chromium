set(SOURCES
	main.c
)
add_executable(ARB_multitexture2 ${SOURCES})
target_link_libraries(ARB_multitexture2 crutil crextensionscommon)

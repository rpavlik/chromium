set(SOURCES
	main.c
)
add_executable(NV_fog_distance ${SOURCES})
target_link_libraries(NV_fog_distance crutil crextensionscommon)

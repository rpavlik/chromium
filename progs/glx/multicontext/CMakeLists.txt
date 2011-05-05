set(SOURCES
	multicontext.c
)
add_executable(multicontext ${SOURCES})
target_link_libraries(multicontext crutil crextensionscommon)

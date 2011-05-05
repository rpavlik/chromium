set(SOURCES
	logo.c
	util.c
)
add_library(crextensionscommon SHARED ${SOURCES})
target_link_libraries(crextensionscommon crutil)

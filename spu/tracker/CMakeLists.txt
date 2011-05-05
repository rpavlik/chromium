set(SOURCES
	trackerspu.c
	trackerspu_config.c
	trackerspu_init.c
	trackerspu_udp.c
)
add_library(trackerspu SHARED ${SOURCES})
target_link_libraries(trackerspu spuload crutil crmothership)

set(SOURCES
	windowtrackerspu.c
	windowtrackerspu_config.c
	windowtrackerspu_init.c
)
add_library(windowtrackerspu SHARED ${SOURCES})
target_link_libraries(windowtrackerspu spuload crutil crmothership)

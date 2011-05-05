set(SOURCES
	readbackspu.c
	readbackspu_config.c
	readbackspu_init.c
)
add_library(readbackspu SHARED ${SOURCES})
target_link_libraries(readbackspu spuload crutil crmothership)

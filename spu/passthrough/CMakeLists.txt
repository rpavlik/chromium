set(SOURCES
	passthroughspu.c
	passthroughspu_init.c
)
add_library(passthroughspu SHARED ${SOURCES})
target_link_libraries(passthroughspu crutil)

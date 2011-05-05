set(SOURCES
	passthroughspu.c
	passthroughspu_init.c
)
set(GENERATED
	passthroughspu.c
)
# TODO: generate these files!


add_library(passthroughspu SHARED ${SOURCES})
target_link_libraries(passthroughspu crutil)

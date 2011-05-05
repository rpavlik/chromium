set(SOURCES
	replicatespu.c
	replicatespu_beginend.c
	replicatespu_bufferobject.c
	replicatespu_client.c
	replicatespu_config.c
	replicatespu_context.c
	replicatespu_diffapi.c
	replicatespu_generate.c
	replicatespu_getstring.c
	replicatespu_init.c
	replicatespu_lists.c
	replicatespu_misc.c
	replicatespu_net.c
	replicatespu_redirect.c
	replicatespu_stateapi.c
	replicatespu_swapbuf.c
	replicatespu_pixel.c
)
set(GENERATED
	replicatespu.c
	replicatespu_beginend.c
	replicatespu_proto.h
	replicatespu_diffapi.c
	replicatespu_generate.c
	replicatespu_stateapi.c
)
# TODO: generate these files!


add_library(replicatespu SHARED ${SOURCES})
target_link_libraries(replicatespu crutil spuload crmothership crstate crdlm replicatespu_crpacker_copy)

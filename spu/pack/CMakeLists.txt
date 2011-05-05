set(SOURCES
	packspu.c
	packspu_beginend.c
	packspu_bufferobject.c
	packspu_client.c
	packspu_config.c
	packspu_context.c
	packspu_get.c
	packspu_getstring.c
	packspu_flush.c
	packspu_init.c
	packspu_misc.c
	packspu_net.c
	packspu_swapbuf.c
	packspu_pixel.c
)
add_library(packspu SHARED ${SOURCES})
target_link_libraries(packspu crutil spuload crmothership crstate packspu_crpacker_copy)

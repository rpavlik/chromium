set(SOURCES
	server_main.c
	server_boundsinfo.c
	server_bufferobject.c
	server_clear.c
	server_clip.c
	server_config.c
	server_context.c
	server_dispatch.c
	server_gentextures.c
	server_get.c
	server_getmap.c
	server_getstring.c
	server_getpointer.c
	server_getpixelmap.c
	server_getteximage.c
	server_lists.c
	server_misc.c
	server_occlude.c
	server_papi.c
	server_projmatrix.c
	server_readpixels.c
	server_retval.c
	server_simpleget.c
	server_stream.c
	server_tiles.c
	server_viewport.c
	server_window.c
	server_winpos.c
	server_writeback.c
)
set(GENERATED
	server_dispatch.h
	server_dispatch.c
	server_get.c
	server_retval.c
	server_simpleget.c
)
# TODO: generate these files!


add_library(crserverlib STATIC ${SOURCES})

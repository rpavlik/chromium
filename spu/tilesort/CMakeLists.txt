set(SOURCES
	tilesortspu.c
	tilesortspu_bcast.c
	tilesortspu_beginend.c
	tilesortspu_bucket.c
	tilesortspu_bufferobject.c
	tilesortspu_client.c
	tilesortspu_config.c
	tilesortspu_context.c
	tilesortspu_copytex.c
	tilesortspu_diffapi.c
	tilesortspu_dlpack_gen.c
	tilesortspu_dlpacker_gen.c
	tilesortspu_dlstate_gen.c
	tilesortspu_eval.c
	tilesortspu_flush.c
	tilesortspu_gen.c
	tilesortspu_get.c
	tilesortspu_init.c
	tilesortspu_lists.c
	tilesortspu_misc.c
	tilesortspu_material.c
	tilesortspu_net.c
	tilesortspu_occlude.c
	tilesortspu_pinch_gen.c
	tilesortspu_pixels.c
	tilesortspu_rect.c
	tilesortspu_stereo.c
	tilesortspu_swap.c
	tilesortspu_tiles.c
	tilesortspu_viewport.c
	tilesortspu_window.c
)
set(GENERATED
	tilesortspu_gen.h
	tilesortspu_gen.c
	tilesortspu.c
	tilesortspu_diffapi.c
	tilesortspu_pinch_gen.c
	tilesortspu_dlpack_gen.c
	tilesortspu_dlpack_gen.h
	tilesortspu_dlstate_gen.c
	tilesortspu_dlstate_gen.h
	tilesortspu_dlpacker_gen.c
)
# TODO: generate these files!


add_library(tilesortspu SHARED ${SOURCES})
target_link_libraries(tilesortspu crutil crmothership crdlm spuload crstate)

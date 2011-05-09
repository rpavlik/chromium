include_directories(.)
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


set(SOURCES
	tilesortspu_bcast.c
	tilesortspu_beginend.c
	tilesortspu_bucket.c
	tilesortspu_bufferobject.c
	tilesortspu_client.c
	tilesortspu_config.c
	tilesortspu_context.c
	tilesortspu_copytex.c
	tilesortspu_eval.c
	tilesortspu_flush.c
	tilesortspu_get.c
	tilesortspu_init.c
	tilesortspu_lists.c
	tilesortspu_misc.c
	tilesortspu_material.c
	tilesortspu_net.c
	tilesortspu_occlude.c
	tilesortspu_pixels.c
	tilesortspu_rect.c
	tilesortspu_stereo.c
	tilesortspu_swap.c
	tilesortspu_tiles.c
	tilesortspu_viewport.c
	tilesortspu_window.c
	${CMAKE_CURRENT_BINARY_DIR}/tilesortspu_gen.h
	${CMAKE_CURRENT_BINARY_DIR}/tilesortspu_gen.c
	${CMAKE_CURRENT_BINARY_DIR}/tilesortspu.c
	${CMAKE_CURRENT_BINARY_DIR}/tilesortspu_diffapi.c
	${CMAKE_CURRENT_BINARY_DIR}/tilesortspu_pinch_gen.c
	${CMAKE_CURRENT_BINARY_DIR}/tilesortspu_dlpack_gen.c
	${CMAKE_CURRENT_BINARY_DIR}/tilesortspu_dlpack_gen.h
	${CMAKE_CURRENT_BINARY_DIR}/tilesortspu_dlstate_gen.c
	${CMAKE_CURRENT_BINARY_DIR}/tilesortspu_dlstate_gen.h
	${CMAKE_CURRENT_BINARY_DIR}/tilesortspu_dlpacker_gen.c
	tilesort.def
)
add_library(tilesortspu SHARED ${SOURCES})
target_link_libraries(tilesortspu crutil crmothership crdlm spuload crstate tilesortspu_crpacker_copy)
target_link_libraries(tilesortspu ${EXTRA_LIBS})
install(TARGETS tilesortspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

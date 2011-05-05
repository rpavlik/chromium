set(SOURCES
	unpack.c
	unpack_arrays.c
	unpack_bounds.c
	unpack_bufferobject.c
	unpack_calllists.c
	unpack_clipplane.c
	unpack_context.c
	unpack_drawpixels.c
	unpack_fence.c
	unpack_fog.c
	unpack_lights.c
	unpack_map.c
	unpack_materials.c
	unpack_matrices.c
	unpack_misc.c
	unpack_pixelmap.c
	unpack_point.c
	unpack_program.c
	unpack_readpixels.c
	unpack_regcombiner.c
	unpack_stipple.c
	unpack_texture.c
	unpack_writeback.c
)
set(GENERATED
	unpack.c
	unpack_extend.h
)
# TODO: generate these files!


add_library(crunpacker SHARED ${SOURCES})
target_link_libraries(crunpacker crutil spuload crpacker)

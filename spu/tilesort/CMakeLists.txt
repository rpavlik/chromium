include_directories(.)

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
)
generate_with_python_arg(generator.py table tilesortspu.c tilesort_special tilesort_unimplemented_special $(APIFILES) tilesortspu_gen.h)
generate_with_python_arg(generator.py header tilesortspu_gen.h tilesort_special tilesort_unimplemented_special $(APIFILES))
generate_with_python_arg(generator.py source tilesortspu_gen.c tilesort_special tilesort_unimplemented_special $(APIFILES) tilesortspu_gen.h)

generate_with_python_arg(dlpack_generator.py table tilesortspu_dlpacker_gen.c tilesort_dlpack_ignore_special tilesortspu_dlpack_gen.h $(APIFILES))
generate_with_python_arg(dlpack_generator.py header tilesortspu_dlpack_gen.h tilesort_dlpack_ignore_special $(APIFILES))
generate_with_python_arg(dlpack_generator.py source tilesortspu_dlpack_gen.c tilesort_dlpack_ignore_special tilesortspu_dlpack_gen.h $(APIFILES))

generate_with_python_arg(dlstate_generator.py table tilesortspu_dlstate_gen.c tilesortspu_dlstate_gen.h $(APIFILES))
generate_with_python_arg(dlstate_generator.py header tilesortspu_dlstate_gen.h $(APIFILES))

generate_with_python(diffapi.py
	tilesortspu_diffapi.c
	${APIFILES})

#TODO don't just use cat here
set(output ${CMAKE_CURRENT_BINARY_DIR}/tilesortspu_pinch_gen.c)
add_custom_command(OUTPUT
		${output}
		COMMAND
		${PYTHON_EXECUTABLE} ../../state_tracker/convert.py > ${output}
		COMMAND
		${PYTHON_EXECUTABLE} pinch_convert.py >> ${output}
		COMMAND
		${PYTHON_EXECUTABLE} pinch_convert.py >> ${output}
		COMMAND
		${PYTHON_EXECUTABLE} length_table.py >> ${output}
		COMMAND
		cat tilesortspu_pinch.c >> ${output}
		WORKING_DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/tilesortspu_pinch.c
		${CMAKE_CURRENT_SOURCE_DIR}/../../state_tracker/convert.py
		${CMAKE_CURRENT_SOURCE_DIR}/length_table.py
		${CMAKE_CURRENT_SOURCE_DIR}/pinch_convert.py
		${ARGN})
add_library(tilesortspu SHARED ${SOURCES})
target_link_libraries(tilesortspu crutil crmothership crdlm spuload crstate tilesortspu_crpacker_copy)
target_link_libraries(tilesortspu ${EXTRA_LIBS})

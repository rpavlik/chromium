include_directories(.)
set(GENERATED
	packer.def
	../include/cr_packfunctions.h
	../include/cr_opcodes.h
	../include/state/cr_currentpointers.h
	pack_bbox.c
	pack_current.c
	packer.c
	pack_arrays_swap.c
	pack_bounds_swap.c
	pack_bufferobject_swap.c
	pack_client_swap.c
	pack_clipplane_swap.c
	pack_fog_swap.c
	pack_lights_swap.c
	pack_materials_swap.c
	pack_matrices_swap.c
	pack_misc_swap.c
	pack_pixels_swap.c
	pack_point_swap.c
	pack_program_swap.c
	pack_regcombiner_swap.c
	pack_stipple_swap.c
)
# TODO: generate these files!


set(SOURCES
	pack_arrays.c
	pack_beginend.c
	pack_bounds.c
	pack_buffer.c
	pack_bufferobject.c
	pack_client.c
	pack_clipplane.c
	pack_context.c
	pack_error.c
	pack_extensions.c
	pack_feedback.c
	pack_fence.c
	pack_fog.c
	pack_init.c
	pack_lights.c
	pack_lists.c
	pack_swap_lists.c
	pack_map.c
	pack_point.c
	pack_swap_map.c
	pack_misc.c
	pack_materials.c
	pack_matrices.c
	pack_pixels.c
	pack_pixelmap.c
	pack_swap_pixelmap.c
	pack_program.c
	pack_regcombiner.c
	pack_stipple.c
	pack_texture.c
	pack_swap_texture.c
	${CMAKE_CURRENT_BINARY_DIR}/packer.def
	${PROJECT_BINARY_DIR}/include/cr_packfunctions.h
	${PROJECT_BINARY_DIR}/include/cr_opcodes.h
	${PROJECT_BINARY_DIR}/include/state/cr_currentpointers.h
	${CMAKE_CURRENT_BINARY_DIR}/pack_bbox.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_current.c
	${CMAKE_CURRENT_BINARY_DIR}/packer.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_arrays_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_bounds_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_bufferobject_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_client_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_clipplane_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_fog_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_lights_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_materials_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_matrices_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_misc_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_pixels_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_point_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_program_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_regcombiner_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/pack_stipple_swap.c
)

generate_with_python(packer_defs.py packer.def ${APIFILES})
generate_with_python(pack_header.py ${PROJECT_BINARY_DIR}/include/cr_packfunctions.h packer_special ${APIFILES})
generate_with_python(opcodes.py ${PROJECT_BINARY_DIR}/include/cr_opcodes.h ${APIFILES})
generate_with_python(pack_currentheader.py ${PROJECT_BINARY_DIR}/include/state/cr_currentpointers.h pack_currenttypes.py)
generate_with_python(packer_bbox.py pack_bbox.c)
generate_with_python(pack_current.py pack_current.c pack_currenttypes.py)
generate_with_python(packer.py packer.c packer_special ${APIFILES})

set(SWAP
	pack_arrays_swap.c
	pack_bounds_swap.c
	pack_bufferobject_swap.c
	pack_client_swap.c
	pack_clipplane_swap.c
	pack_fog_swap.c
	pack_lights_swap.c
	pack_materials_swap.c
	pack_matrices_swap.c
	pack_misc_swap.c
	pack_pixels_swap.c
	pack_point_swap.c
	pack_program_swap.c
	pack_regcombiner_swap.c
	pack_stipple_swap.c)

foreach(SWAPSOURCE ${SWAP})
	string(REPLACE "_swap" "" ORIGSOURCE ${SWAPSOURCE})
	add_custom_command(OUTPUT ${SWAPSOURCE}
		COMMAND
		${PYTHON_EXECUTABLE} pack_swap.py ${ORIGSOURCE} > ${CMAKE_CURRENT_BINARY_DIR}/${SWAPSOURCE}
		WORKING_DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/pack_swap.py
		${CMAKE_CURRENT_SOURCE_DIR}/${ORIGSOURCE})
endforeach()

add_library(crpacker SHARED ${SOURCES})
target_link_libraries(crpacker crutil)
target_link_libraries(crpacker ${EXTRA_LIBS})
install(TARGETS crpacker
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
add_library(packspu_crpacker_copy SHARED ${SOURCES})
target_link_libraries(packspu_crpacker_copy crutil)
target_link_libraries(packspu_crpacker_copy ${EXTRA_LIBS})
install(TARGETS packspu_crpacker_copy
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
add_library(tilesortspu_crpacker_copy SHARED ${SOURCES})
target_link_libraries(tilesortspu_crpacker_copy crutil)
target_link_libraries(tilesortspu_crpacker_copy ${EXTRA_LIBS})
install(TARGETS tilesortspu_crpacker_copy
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
add_library(hiddenlinespu_crpacker_copy SHARED ${SOURCES})
target_link_libraries(hiddenlinespu_crpacker_copy crutil)
target_link_libraries(hiddenlinespu_crpacker_copy ${EXTRA_LIBS})
install(TARGETS hiddenlinespu_crpacker_copy
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
add_library(archsplitspu_crpacker_copy SHARED ${SOURCES})
target_link_libraries(archsplitspu_crpacker_copy crutil)
target_link_libraries(archsplitspu_crpacker_copy ${EXTRA_LIBS})
install(TARGETS archsplitspu_crpacker_copy
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)
add_library(replicatespu_crpacker_copy SHARED ${SOURCES})
target_link_libraries(replicatespu_crpacker_copy crutil)
target_link_libraries(replicatespu_crpacker_copy ${EXTRA_LIBS})
install(TARGETS replicatespu_crpacker_copy
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

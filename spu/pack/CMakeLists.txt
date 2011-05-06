include_directories(.)
set(GENERATED
	packspu.c
	packspu_get.c
	packspu_flush.c
	packspu_beginend.c
	packspu_proto.h
)
# TODO: generate these files!


set(SOURCES
	packspu_bufferobject.c
	packspu_client.c
	packspu_config.c
	packspu_context.c
	packspu_getstring.c
	packspu_init.c
	packspu_misc.c
	packspu_net.c
	packspu_swapbuf.c
	packspu_pixel.c
	${CMAKE_CURRENT_BINARY_DIR}/packspu.c
	${CMAKE_CURRENT_BINARY_DIR}/packspu_get.c
	${CMAKE_CURRENT_BINARY_DIR}/packspu_flush.c
	${CMAKE_CURRENT_BINARY_DIR}/packspu_beginend.c
	${CMAKE_CURRENT_BINARY_DIR}/packspu_proto.h
)

generate_with_python(packspu_proto.py
	packspu_proto.h
	packspu_special packspu_unimplemented_special
	${APIFILES})
generate_with_python(pack.py
	packspu.c
	packspu_special packspu_unimplemented_special ${CMAKE_CURRENT_BINARY_DIR}/packspu_proto.h
	${APIFILES})
generate_with_python(packspu_get.py
	packspu_get.c
	packspu_special ${PROJECT_SOURCE_DIR}/crserverlib/get_sizes.py ${CMAKE_CURRENT_BINARY_DIR}/packspu_proto.h
	${APIFILES})
generate_with_python(packspu_beginend.py
	packspu_beginend.c
	packspu_vertex_special ${CMAKE_CURRENT_BINARY_DIR}/packspu_proto.h
	${APIFILES})
generate_with_python(packspu_flush.py
	packspu_flush.c
	packspu_flush_special
	${APIFILES})
add_library(packspu SHARED ${SOURCES})
target_link_libraries(packspu crutil spuload crmothership crstate packspu_crpacker_copy)
target_link_libraries(packspu ${EXTRA_LIBS})

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
	pack.def
)
add_library(packspu SHARED ${SOURCES})
target_link_libraries(packspu crutil spuload crmothership crstate packspu_crpacker_copy)
target_link_libraries(packspu ${EXTRA_LIBS})
install(TARGETS packspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

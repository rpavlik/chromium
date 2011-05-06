include_directories(.)
set(GENERATED
	hiddenlinespu.c
	hiddenlinespu_pixel.c
	hiddenlinespu_proto.h
)
# TODO: generate these files!


set(SOURCES
	hiddenlinespu_buffer.c
	hiddenlinespu_clear.c
	hiddenlinespu_clearcolor.c
	hiddenlinespu_config.c
	hiddenlinespu_context.c
	hiddenlinespu_depthmask.c
	hiddenlinespu_disable.c
	hiddenlinespu_init.c
	hiddenlinespu_swap.c
	${CMAKE_CURRENT_BINARY_DIR}/hiddenlinespu.c
	${CMAKE_CURRENT_BINARY_DIR}/hiddenlinespu_pixel.c
	${CMAKE_CURRENT_BINARY_DIR}/hiddenlinespu_proto.h
)
add_library(hiddenlinespu SHARED ${SOURCES})
target_link_libraries(hiddenlinespu spuload crutil crmothership crpacker crstate hiddenlinespu_crpacker_copy hiddenlinespu_crunpacker_copy)
target_link_libraries(hiddenlinespu ${EXTRA_LIBS})
install(TARGETS hiddenlinespu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

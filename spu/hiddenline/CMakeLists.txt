set(SOURCES
	hiddenlinespu.c
	hiddenlinespu_buffer.c
	hiddenlinespu_clear.c
	hiddenlinespu_clearcolor.c
	hiddenlinespu_config.c
	hiddenlinespu_context.c
	hiddenlinespu_depthmask.c
	hiddenlinespu_disable.c
	hiddenlinespu_init.c
	hiddenlinespu_pixel.c
	hiddenlinespu_swap.c
)
set(GENERATED
	hiddenlinespu.c
	hiddenlinespu_pixel.c
	hiddenlinespu_proto.h
)
# TODO: generate these files!


add_library(hiddenlinespu SHARED ${SOURCES})
target_link_libraries(hiddenlinespu spuload crutil crmothership crpacker crstate)

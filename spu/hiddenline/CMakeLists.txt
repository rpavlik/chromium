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
add_library(hiddenlinespu SHARED ${SOURCES})
target_link_libraries(hiddenlinespu spuload crutil crmothership crpacker crstate hiddenlinespu_crpacker_copy hiddenlinespu_crunpacker_copy)

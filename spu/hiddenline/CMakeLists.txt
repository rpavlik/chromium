include_directories(.)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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
generate_with_python(hiddenline.py
	hiddenlinespu.c
	hiddenline_special
	hiddenline_pixel_special
	${CMAKE_CURRENT_BINARY_DIR}/hiddenlinespu_proto.h
	${APIFILES})
generate_with_python(hiddenlinespu_pixel.py
	hiddenlinespu_pixel.c
	hiddenline_special
	${APIFILES})
generate_with_python(hiddenlinespu_proto.py
	hiddenlinespu_proto.h
	hiddenline_special
	hiddenline_pixel_special
	${APIFILES})
add_library(hiddenlinespu SHARED ${SOURCES})
target_link_libraries(hiddenlinespu spuload crutil crmothership crpacker crstate hiddenlinespu_crpacker_copy hiddenlinespu_crunpacker_copy)
target_link_libraries(hiddenlinespu ${EXTRA_LIBS})
install(TARGETS hiddenlinespu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

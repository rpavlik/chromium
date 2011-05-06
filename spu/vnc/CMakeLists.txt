include_directories(.)
set(SOURCES
	vncspu.c
	vncspu_config.c
	vncspu_init.c
	actions.c
	active.c
	async_io.c
	buffer.c
	client_io.c
	control.c
	d3des.c
	decode_hextile.c
	decode_tight.c
	encode.c
	encode_tight.c
	fbs_files.c
	host_connect.c
	host_io.c
	logging.c
	main.c
	region.c
	region_more.c
	rfblib.c
	translate.c
)
add_library(vncspu SHARED ${SOURCES})
target_link_libraries(vncspu spuload crutil crmothership)
target_link_libraries(vncspu ${EXTRA_LIBS})

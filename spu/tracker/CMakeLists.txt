include_directories(.)
set(SOURCES
	trackerspu.c
	trackerspu_config.c
	trackerspu_init.c
	trackerspu_udp.c
	trackerspu.def
)
add_library(trackerspu SHARED ${SOURCES})
target_link_libraries(trackerspu spuload crutil crmothership)
target_link_libraries(trackerspu ${EXTRA_LIBS})
install(TARGETS trackerspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

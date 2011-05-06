include_directories(.)
set(SOURCES
	getconfig.c
)
add_executable(dmx_config  ${SOURCES})
target_link_libraries(dmx_config ${EXTRA_LIBS})

include_directories(.)
set(SOURCES
	getconfig.c
)
add_executable(dmx_config  ${SOURCES})
target_link_libraries(dmx_config ${EXTRA_LIBS})
install(TARGETS dmx_config
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

include_directories(.)
set(SOURCES
	perfspu.c
	perfspu_config.c
	perfspu_init.c
	perfspu.def
)
add_library(perfspu SHARED ${SOURCES})
target_link_libraries(perfspu spuload crutil crmothership)
target_link_libraries(perfspu ${EXTRA_LIBS})
install(TARGETS perfspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

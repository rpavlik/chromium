include_directories(.)
set(SOURCES
	windowtrackerspu.c
	windowtrackerspu_config.c
	windowtrackerspu_init.c
	windowtrackerspu.def
)
add_library(windowtrackerspu SHARED ${SOURCES})
target_link_libraries(windowtrackerspu spuload crutil crmothership)
target_link_libraries(windowtrackerspu ${EXTRA_LIBS})
install(TARGETS windowtrackerspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

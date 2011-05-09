include_directories(.)
set(SOURCES
	grabberspu.c
	grabberspu_config.c
	grabberspu_init.c
	grabber.def
)
add_library(grabberspu SHARED ${SOURCES})
target_link_libraries(grabberspu crutil crmothership spuload)
target_link_libraries(grabberspu ${EXTRA_LIBS})
install(TARGETS grabberspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

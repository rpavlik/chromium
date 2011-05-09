include_directories(.)
set(SOURCES
	renderspu.c
	renderspu_config.c
	renderspu_init.c
	renderspu_glx.c
	render.def
)
add_library(renderspu SHARED ${SOURCES})
target_link_libraries(renderspu crutil crmothership spuload)
target_link_libraries(renderspu ${EXTRA_LIBS})
install(TARGETS renderspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

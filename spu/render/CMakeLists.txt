set(SOURCES
	renderspu.c
	renderspu_config.c
	renderspu_init.c
	renderspu_glx.c
)
add_library(renderspu SHARED ${SOURCES})
target_link_libraries(renderspu crutil crmothership spuload)

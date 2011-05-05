set(SOURCES
	expandospu.c
	expandospu_config.c
	expandospu_init.c
	expando.c
)
add_library(expandospu SHARED ${SOURCES})
target_link_libraries(expandospu spuload crutil crmothership crdlm crstate)

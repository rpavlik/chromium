set(SOURCES
	expandospu.c
	expandospu_config.c
	expandospu_init.c
	expando.c
)
set(GENERATED
	expando.c
)
# TODO: generate these files!


add_library(expandospu SHARED ${SOURCES})
target_link_libraries(expandospu spuload crutil crmothership crdlm crstate)

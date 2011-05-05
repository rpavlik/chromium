set(SOURCES
	simplequeryspu.c
	simplequeryspu_config.c
	simplequeryspu_init.c
)
set(GENERATED
	simplequeryspu.c
)
# TODO: generate these files!


add_library(simplequeryspu SHARED ${SOURCES})
target_link_libraries(simplequeryspu spuload crutil crmothership crstate)

set(SOURCES
	simplequeryspu.c
	simplequeryspu_config.c
	simplequeryspu_init.c
)
add_library(simplequeryspu SHARED ${SOURCES})
target_link_libraries(simplequeryspu spuload crutil crmothership crstate)

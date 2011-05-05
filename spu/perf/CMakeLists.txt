set(SOURCES
	perfspu.c
	perfspu_config.c
	perfspu_init.c
)
add_library(perfspu SHARED ${SOURCES})
target_link_libraries(perfspu spuload crutil crmothership)

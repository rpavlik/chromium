set(SOURCES
	zpixspu_config.c
	zpixspu_init.c
	zpixspu.c
)
add_library(zpixspu SHARED ${SOURCES})
target_link_libraries(zpixspu spuload crutil crmothership)

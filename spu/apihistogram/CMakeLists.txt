set(SOURCES
	apihistogramspu.c
	apihistogramspu_config.c
	apihistogramspu_init.c
)
add_library(apihistogramspu SHARED ${SOURCES})
target_link_libraries(apihistogramspu spuload crutil crmothership)

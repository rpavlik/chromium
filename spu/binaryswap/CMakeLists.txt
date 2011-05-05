set(SOURCES
	binaryswapspu.c
	binaryswapspu_config.c
	binaryswapspu_init.c
)
add_library(binaryswapspu SHARED ${SOURCES})
target_link_libraries(binaryswapspu spuload crutil crmothership)

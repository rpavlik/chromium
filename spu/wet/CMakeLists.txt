set(SOURCES
	wetspu.c
	wetspu_config.c
	wetspu_init.c
)
add_library(wetspu SHARED ${SOURCES})
target_link_libraries(wetspu spuload crutil crmothership)

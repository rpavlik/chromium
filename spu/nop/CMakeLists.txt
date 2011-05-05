set(SOURCES
	nopspu.c
	nopspu_init.c
	nopspu_config.c
)
add_library(nopspu SHARED ${SOURCES})
target_link_libraries(nopspu crmothership crutil spuload crstate)

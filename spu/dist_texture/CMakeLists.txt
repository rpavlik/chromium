set(SOURCES
	dist_texturespu.c
	dist_texturespu_config.c
	dist_texturespu_texture.c
	dist_texturespu_init.c
)
add_library(dist_texturespu SHARED ${SOURCES})
target_link_libraries(dist_texturespu spuload crutil crmothership)

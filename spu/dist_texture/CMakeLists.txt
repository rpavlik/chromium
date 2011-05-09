include_directories(.)
set(SOURCES
	dist_texturespu.c
	dist_texturespu_config.c
	dist_texturespu_texture.c
	dist_texturespu_init.c
	dist_texturespu.def
)
add_library(dist_texturespu SHARED ${SOURCES})
target_link_libraries(dist_texturespu spuload crutil crmothership)
target_link_libraries(dist_texturespu ${EXTRA_LIBS})
install(TARGETS dist_texturespu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

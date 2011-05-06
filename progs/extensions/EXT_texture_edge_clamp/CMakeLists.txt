include_directories(.)
set(SOURCES
	main.c
)
add_executable(EXT_texture_edge_clamp  ${SOURCES})
target_link_libraries(EXT_texture_edge_clamp crutil crextensionscommon)
target_link_libraries(EXT_texture_edge_clamp ${EXTRA_LIBS})
install(TARGETS EXT_texture_edge_clamp
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

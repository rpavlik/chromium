include_directories(.)
set(SOURCES
	main.c
)
add_executable(EXT_texture_filter_anisotropic  ${SOURCES})
target_link_libraries(EXT_texture_filter_anisotropic crutil crextensionscommon)
target_link_libraries(EXT_texture_filter_anisotropic ${EXTRA_LIBS})
install(TARGETS EXT_texture_filter_anisotropic
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

include_directories(.)
set(SOURCES
	main.c
)
add_executable(ARB_texture_cube_map  ${SOURCES})
target_link_libraries(ARB_texture_cube_map crutil crextensionscommon)
target_link_libraries(ARB_texture_cube_map ${EXTRA_LIBS})
install(TARGETS ARB_texture_cube_map
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

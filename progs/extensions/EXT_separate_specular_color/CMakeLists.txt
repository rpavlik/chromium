include_directories(.)
set(SOURCES
	main.c
)
add_executable(EXT_separate_specular_color  ${SOURCES})
target_link_libraries(EXT_separate_specular_color crutil crextensionscommon)
target_link_libraries(EXT_separate_specular_color ${EXTRA_LIBS})
install(TARGETS EXT_separate_specular_color
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

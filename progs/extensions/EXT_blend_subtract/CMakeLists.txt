include_directories(.)
set(SOURCES
	main.c
)
add_executable(EXT_blend_subtract  ${SOURCES})
target_link_libraries(EXT_blend_subtract crutil crextensionscommon)
target_link_libraries(EXT_blend_subtract ${EXTRA_LIBS})
install(TARGETS EXT_blend_subtract
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

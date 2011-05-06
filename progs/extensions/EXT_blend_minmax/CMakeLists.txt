include_directories(.)
set(SOURCES
	main.c
)
add_executable(EXT_blend_minmax  ${SOURCES})
target_link_libraries(EXT_blend_minmax crutil crextensionscommon)
target_link_libraries(EXT_blend_minmax ${EXTRA_LIBS})
install(TARGETS EXT_blend_minmax
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

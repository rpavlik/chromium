include_directories(.)
set(SOURCES
	main.c
)
add_executable(ARB_multitexture2  ${SOURCES})
target_link_libraries(ARB_multitexture2 crutil crextensionscommon)
target_link_libraries(ARB_multitexture2 ${EXTRA_LIBS})
install(TARGETS ARB_multitexture2
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

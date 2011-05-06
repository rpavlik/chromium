include_directories(.)
set(SOURCES
	main.c
)
add_executable(ARB_multitexture  ${SOURCES})
target_link_libraries(ARB_multitexture crutil crextensionscommon)
target_link_libraries(ARB_multitexture ${EXTRA_LIBS})
install(TARGETS ARB_multitexture
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

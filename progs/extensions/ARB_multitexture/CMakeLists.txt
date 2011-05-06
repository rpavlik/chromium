include_directories(.)
set(SOURCES
	main.c
)
add_executable(ARB_multitexture  ${SOURCES})
target_link_libraries(ARB_multitexture crutil crextensionscommon)
target_link_libraries(ARB_multitexture ${EXTRA_LIBS})

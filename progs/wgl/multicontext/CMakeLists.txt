include_directories(.)
set(SOURCES
	multicontext.c
)
add_executable(multicontext  ${SOURCES})
target_link_libraries(multicontext ../../../built/crextensionscommon/Linux/libcrextensionscommon.a)
target_link_libraries(multicontext ${EXTRA_LIBS})
install(TARGETS multicontext
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

include_directories(.)
set(SOURCES
	main.c
)
add_executable(NV_fog_distance  ${SOURCES})
target_link_libraries(NV_fog_distance crutil crextensionscommon)
target_link_libraries(NV_fog_distance ${EXTRA_LIBS})
install(TARGETS NV_fog_distance
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

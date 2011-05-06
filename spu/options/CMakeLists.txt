include_directories(.)
set(SOURCES
	spuoptions.c
)
add_executable(spuoptions  ${SOURCES})
target_link_libraries(spuoptions spuload crutil crmothership)
target_link_libraries(spuoptions ${EXTRA_LIBS})
install(TARGETS spuoptions
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

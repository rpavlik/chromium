include_directories(.)
set(SOURCES
	sputest.c
)
add_executable(sputest  ${SOURCES})
target_link_libraries(sputest spuload crutil crmothership)
target_link_libraries(sputest ${EXTRA_LIBS})
install(TARGETS sputest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

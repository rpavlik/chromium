include_directories(.)
set(SOURCES
	statecopytest.c
)
add_executable(statecopytest  ${SOURCES})
target_link_libraries(statecopytest crutil spuload crmothership crstate)
target_link_libraries(statecopytest ${EXTRA_LIBS})
install(TARGETS statecopytest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

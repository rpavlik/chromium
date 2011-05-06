include_directories(.)
set(SOURCES
	client.c
)
add_library(crmothership SHARED ${SOURCES})
target_link_libraries(crmothership crutil)
target_link_libraries(crmothership ${EXTRA_LIBS})
install(TARGETS crmothership
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

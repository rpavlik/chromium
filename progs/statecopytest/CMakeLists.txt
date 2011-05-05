set(SOURCES
	statecopytest.c
)
add_executable(statecopytest ${SOURCES})
target_link_libraries(statecopytest crutil spuload crmothership crstate)

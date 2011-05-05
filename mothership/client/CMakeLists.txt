set(SOURCES
	client.c
)
add_library(crmothership SHARED ${SOURCES})
target_link_libraries(crmothership crutil)

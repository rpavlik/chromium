set(SOURCES
	npclient.c
)
add_executable(npclient ${SOURCES})
target_link_libraries(npclient crmothership crutil)

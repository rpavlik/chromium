include_directories(.)
set(SOURCES
	npclient.c
)
add_executable(npclient  ${SOURCES})
target_link_libraries(npclient crmothership crutil)
target_link_libraries(npclient ${EXTRA_LIBS})

set(SOURCES
	main.c
)
add_executable(crserver ${SOURCES})
target_link_libraries(crserver crserverlib spuload crmothership crstate crpacker crutil)

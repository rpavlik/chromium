set(SOURCES
	reset.c
)
add_executable(resetms ${SOURCES})
target_link_libraries(resetms crmothership crutil)

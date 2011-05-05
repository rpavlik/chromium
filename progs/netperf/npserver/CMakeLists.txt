set(SOURCES
	npserver.c
)
add_executable(npserver ${SOURCES})
target_link_libraries(npserver crmothership crutil)

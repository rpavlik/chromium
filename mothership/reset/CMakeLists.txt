include_directories(.)
set(SOURCES
	reset.c
)
add_executable(resetms  ${SOURCES})
target_link_libraries(resetms crmothership crutil)
target_link_libraries(resetms ${EXTRA_LIBS})

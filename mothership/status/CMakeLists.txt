include_directories(.)
set(SOURCES
	status.c
)
add_executable(statusms  ${SOURCES})
target_link_libraries(statusms crmothership crutil)
target_link_libraries(statusms ${EXTRA_LIBS})

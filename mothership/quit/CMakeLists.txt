include_directories(.)
set(SOURCES
	quit.c
)
add_executable(quitms  ${SOURCES})
target_link_libraries(quitms crmothership crutil)
target_link_libraries(quitms ${EXTRA_LIBS})

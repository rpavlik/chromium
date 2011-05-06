include_directories(.)
set(SOURCES
	viewports.c
)
add_executable(viewports  ${SOURCES})
target_link_libraries(viewports ${EXTRA_LIBS})

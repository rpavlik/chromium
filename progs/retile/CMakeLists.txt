include_directories(.)
set(SOURCES
	retile.c
)
add_executable(retile  ${SOURCES})
target_link_libraries(retile ${EXTRA_LIBS})

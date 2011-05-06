include_directories(.)
set(SOURCES
	stereocube.c
)
add_executable(stereocube  ${SOURCES})
target_link_libraries(stereocube ${EXTRA_LIBS})

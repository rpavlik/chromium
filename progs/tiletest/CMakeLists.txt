include_directories(.)
set(SOURCES
	tiletest.c
)
add_executable(tiletest  ${SOURCES})
target_link_libraries(tiletest ${EXTRA_LIBS})

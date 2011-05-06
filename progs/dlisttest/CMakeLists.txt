include_directories(.)
set(SOURCES
	dlisttest.c
)
add_executable(dlisttest  ${SOURCES})
target_link_libraries(dlisttest ${EXTRA_LIBS})

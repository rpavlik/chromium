include_directories(.)
set(SOURCES
	fonttest.c
)
add_executable(fonttest  ${SOURCES})
target_link_libraries(fonttest ${EXTRA_LIBS})

include_directories(.)
set(SOURCES
	nullapp.c
)
add_executable(nullapp  ${SOURCES})
target_link_libraries(nullapp ${EXTRA_LIBS})

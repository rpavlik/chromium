include_directories(.)
set(SOURCES
	city.c
	multiview.c
)
add_executable(city  ${SOURCES})
target_link_libraries(city ${EXTRA_LIBS})

include_directories(.)
set(SOURCES
	bug.c
)
add_executable(kirchner_material  ${SOURCES})
target_link_libraries(kirchner_material ${EXTRA_LIBS})

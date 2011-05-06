include_directories(.)

set(SOURCES
	simplequeryspu_config.c
	simplequeryspu_init.c
	${CMAKE_CURRENT_BINARY_DIR}/simplequeryspu.c
)

generate_with_python(simplequery.py
	simplequeryspu.c
	${APIFILES})

add_library(simplequeryspu SHARED ${SOURCES})
target_link_libraries(simplequeryspu spuload crutil crmothership crstate)
target_link_libraries(simplequeryspu ${EXTRA_LIBS})

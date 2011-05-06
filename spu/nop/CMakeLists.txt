include_directories(.)

set(SOURCES
	nopspu_init.c
	nopspu_config.c
	${CMAKE_CURRENT_BINARY_DIR}/nopspu.c
)

generate_with_python(nop.py nopspu.c nop_special ${APIFILES})

add_library(nopspu SHARED ${SOURCES})
target_link_libraries(nopspu crmothership crutil spuload crstate)
target_link_libraries(nopspu ${EXTRA_LIBS})

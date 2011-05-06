include_directories(.)
set(GENERATED
	nopspu.c
)
# TODO: generate these files!


set(SOURCES
	nopspu_init.c
	nopspu_config.c
	${CMAKE_CURRENT_BINARY_DIR}/nopspu.c
)
add_library(nopspu SHARED ${SOURCES})
target_link_libraries(nopspu crmothership crutil spuload crstate)
target_link_libraries(nopspu ${EXTRA_LIBS})

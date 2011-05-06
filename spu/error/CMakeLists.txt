include_directories(.)

set(SOURCES
	errorspu_init.c
	${CMAKE_CURRENT_BINARY_DIR}/errorspu.c
)

generate_with_python(error.py errorspu.c ${APIFILES})

add_library(errorspu SHARED ${SOURCES})
target_link_libraries(errorspu crutil)
target_link_libraries(errorspu ${EXTRA_LIBS})

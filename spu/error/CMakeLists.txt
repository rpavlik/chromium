set(SOURCES
	errorspu.c
	errorspu_init.c
)
add_library(errorspu SHARED ${SOURCES})
target_link_libraries(errorspu crutil)

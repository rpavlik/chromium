set(SOURCES
	errorspu.c
	errorspu_init.c
)
set(GENERATED
	errorspu.c
)
# TODO: generate these files!


add_library(errorspu SHARED ${SOURCES})
target_link_libraries(errorspu crutil)

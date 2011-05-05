set(SOURCES
	printspu.c
	printspu_config.c
	printspu_enums.c
	printspu_get.c
	printspu_init.c
	printspu_lighting.c
	printspu_lists.c
	printspu_matrices.c
	printspu_misc.c
	printspu_tex.c
)
add_library(printspu SHARED ${SOURCES})
target_link_libraries(printspu crutil crmothership spuload)

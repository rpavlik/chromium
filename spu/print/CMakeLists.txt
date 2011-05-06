include_directories(.)
set(GENERATED
	printspu.c
)
# TODO: generate these files!


set(SOURCES
	printspu_config.c
	printspu_enums.c
	printspu_get.c
	printspu_init.c
	printspu_lighting.c
	printspu_lists.c
	printspu_matrices.c
	printspu_misc.c
	printspu_tex.c
	${CMAKE_CURRENT_BINARY_DIR}/printspu.c
)
add_library(printspu SHARED ${SOURCES})
target_link_libraries(printspu crutil crmothership spuload)
target_link_libraries(printspu ${EXTRA_LIBS})
install(TARGETS printspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

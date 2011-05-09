include_directories(.)

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
	print.def
)
generate_with_python(print.py printspu.c printspu_unimplemented_special printspu_special ${APIFILES})

#TODO
#To generate a new printspu_enums.c file, do this:
#	python enums.py /usr/include/GL/gl.h ../../include/GL/glext.h ../../include/chromium.h > printspu_enums.c

add_library(printspu SHARED ${SOURCES})
target_link_libraries(printspu crutil crmothership spuload)
target_link_libraries(printspu ${EXTRA_LIBS})
install(TARGETS printspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

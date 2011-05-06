include_directories(.)
set(GENERATED
	server_dispatch.h
	server_dispatch.c
	server_get.c
	server_retval.c
	server_simpleget.c
)
# TODO: generate these files!


set(SOURCES
	server_main.c
	server_boundsinfo.c
	server_bufferobject.c
	server_clear.c
	server_clip.c
	server_config.c
	server_context.c
	server_gentextures.c
	server_getmap.c
	server_getstring.c
	server_getpointer.c
	server_getpixelmap.c
	server_getteximage.c
	server_lists.c
	server_misc.c
	server_occlude.c
	server_papi.c
	server_projmatrix.c
	server_readpixels.c
	server_stream.c
	server_tiles.c
	server_viewport.c
	server_window.c
	server_winpos.c
	server_writeback.c
	${CMAKE_CURRENT_BINARY_DIR}/server_dispatch.h
	${CMAKE_CURRENT_BINARY_DIR}/server_dispatch.c
	${CMAKE_CURRENT_BINARY_DIR}/server_get.c
	${CMAKE_CURRENT_BINARY_DIR}/server_retval.c
	${CMAKE_CURRENT_BINARY_DIR}/server_simpleget.c
)
# TODO: generate these files!
set(GENERATION_DEPS
	${CMAKE_CURRENT_SOURCE_DIR}/server_special
	${CMAKE_CURRENT_SOURCE_DIR}/../glapi_parser/apiutil.py
	${CMAKE_CURRENT_SOURCE_DIR}/../glapi_parser/APIspec.txt)
add_custom_command(OUTPUT server_dispatch.h
	COMMAND ${PYTHON_EXECUTABLE} server_dispatch_header.py > ${CMAKE_CURRENT_BINARY_DIR}/server_dispatch.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${GENERATION_DEPS} ${CMAKE_CURRENT_SOURCE_DIR}/server_dispatch_header.py)

add_custom_command(OUTPUT server_dispatch.c
	COMMAND ${PYTHON_EXECUTABLE} server_dispatch.py > ${CMAKE_CURRENT_BINARY_DIR}/server_dispatch.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${GENERATION_DEPS} ${CMAKE_CURRENT_SOURCE_DIR}/server_dispatch.py)

add_custom_command(OUTPUT server_retval.c
	COMMAND ${PYTHON_EXECUTABLE} server_retval.py > ${CMAKE_CURRENT_BINARY_DIR}/server_retval.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${GENERATION_DEPS} ${CMAKE_CURRENT_SOURCE_DIR}/server_retval.py)

add_custom_command(OUTPUT server_get.c
	COMMAND ${PYTHON_EXECUTABLE} server_get.py > ${CMAKE_CURRENT_BINARY_DIR}/server_get.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${GENERATION_DEPS} ${CMAKE_CURRENT_SOURCE_DIR}/server_get.py ${CMAKE_CURRENT_SOURCE_DIR}/get_components.py)


add_custom_command(OUTPUT server_simpleget.c
	COMMAND ${PYTHON_EXECUTABLE} server_simpleget.py > ${CMAKE_CURRENT_BINARY_DIR}/server_simpleget.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	VERBATIM
	DEPENDS ${GENERATION_DEPS} ${CMAKE_CURRENT_SOURCE_DIR}/server_simpleget.py ${CMAKE_CURRENT_SOURCE_DIR}/get_sizes.py)

add_library(crserverlib STATIC ${SOURCES})
target_link_libraries(crserverlib ${EXTRA_LIBS})

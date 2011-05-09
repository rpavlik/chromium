include_directories(.)
set(GENERATED
	..//include/spu_dispatch_table.h
	dispatch.c
	spucopy.c
	spuchange.c
	glloader.c
)
# TODO: generate these files!


set(SOURCES
	spuinit.c
	spuload.c
	choosevisual.c
	${PROJECT_BINARY_DIR}/include/spu_dispatch_table.h
	${CMAKE_CURRENT_BINARY_DIR}/dispatch.c
	${CMAKE_CURRENT_BINARY_DIR}/spucopy.c
	${CMAKE_CURRENT_BINARY_DIR}/spuchange.c
	${CMAKE_CURRENT_BINARY_DIR}/glloader.c
	loader.def
)

generate_with_python(dispatchheader.py ${PROJECT_BINARY_DIR}/include/spu_dispatch_table.h ${APIFILES})
generate_with_python(dispatch.py dispatch.c ${APIFILES})
generate_with_python(spucopy.py spucopy.c ${APIFILES})
generate_with_python(spuchange.py spuchange.c ${APIFILES})
generate_with_python(glloader.py glloader.c ${APIFILES})

add_library(spuload SHARED ${SOURCES})
target_link_libraries(spuload crutil crmothership)
target_link_libraries(spuload ${EXTRA_LIBS})
if(X11_PLATFORM)
	target_link_libraries(spuload ${X11_LIBRARIES})
endif()
install(TARGETS spuload
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

add_custom_target(spu_dispatch_table_header ALL
	DEPENDS ${PROJECT_BINARY_DIR}/include/spu_dispatch_table.h)

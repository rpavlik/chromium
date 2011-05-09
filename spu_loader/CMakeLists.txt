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
	${CMAKE_CURRENT_BINARY_DIR}/..//include/spu_dispatch_table.h
	${CMAKE_CURRENT_BINARY_DIR}/dispatch.c
	${CMAKE_CURRENT_BINARY_DIR}/spucopy.c
	${CMAKE_CURRENT_BINARY_DIR}/spuchange.c
	${CMAKE_CURRENT_BINARY_DIR}/glloader.c
	loader.def
)
add_library(spuload SHARED ${SOURCES})
target_link_libraries(spuload crutil crmothership)
target_link_libraries(spuload ${EXTRA_LIBS})
install(TARGETS spuload
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

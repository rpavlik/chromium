include_directories(.)
set(GENERATED
	Linux_x86_64_exports.c
	getprocaddress.c
	opengl.def
	NULLfuncs.c
	tsfuncs.c
)
# TODO: generate these files!


set(SOURCES
	load.c
	stub.c
	context.c
	glx.c
	xfont.c
	${CMAKE_CURRENT_BINARY_DIR}/Linux_x86_64_exports.c
	${CMAKE_CURRENT_BINARY_DIR}/getprocaddress.c
	${CMAKE_CURRENT_BINARY_DIR}/opengl.def
	${CMAKE_CURRENT_BINARY_DIR}/NULLfuncs.c
	${CMAKE_CURRENT_BINARY_DIR}/tsfuncs.c
)
add_library(crfaker SHARED ${SOURCES})
target_link_libraries(crfaker crmothership crutil spuload)
target_link_libraries(crfaker ${EXTRA_LIBS})
install(TARGETS crfaker
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

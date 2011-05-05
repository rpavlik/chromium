set(SOURCES
	getprocaddress.c
	load.c
	stub.c
	context.c
	NULLfuncs.c
	tsfuncs.c
	Linux_x86_64_exports.c
	glx.c
	xfont.c
)
set(GENERATED
	Linux_x86_64_exports.c
	getprocaddress.c
	opengl.def
	NULLfuncs.c
	tsfuncs.c
)
# TODO: generate these files!


add_library(crfaker SHARED ${SOURCES})
target_link_libraries(crfaker crmothership crutil spuload)

set(SOURCES
	spuchange.c
	spucopy.c
	spuinit.c
	spuload.c
	dispatch.c
	glloader.c
	choosevisual.c
)
add_library(spuload SHARED ${SOURCES})
target_link_libraries(spuload crutil crmothership)

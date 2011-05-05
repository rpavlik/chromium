set(SOURCES
	spuchange.c
	spucopy.c
	spuinit.c
	spuload.c
	dispatch.c
	glloader.c
	choosevisual.c
)
set(GENERATED
	..//include/spu_dispatch_table.h
	dispatch.c
	spucopy.c
	spuchange.c
	glloader.c
)
# TODO: generate these files!


add_library(spuload SHARED ${SOURCES})
target_link_libraries(spuload crutil crmothership)

#TODOLINUXSPECIFIC
find_package(X11 REQUIRED)
target_link_libraries(spuload ${X11_LIBRARIES})

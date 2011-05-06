include_directories(.)
set(SOURCES
	disttexture.c
)
add_executable(disttexture  ${SOURCES})
target_link_libraries(disttexture ${EXTRA_LIBS})

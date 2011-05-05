set(SOURCES
	spuoptions.c
)
add_executable(spuoptions ${SOURCES})
target_link_libraries(spuoptions spuload crutil crmothership)

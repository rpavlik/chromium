include_directories(.)
set(SOURCES
	templatespu.c
	templatespu_config.c
	templatespu_init.c
)
add_library(templatespu SHARED ${SOURCES})
target_link_libraries(templatespu spuload crutil crmothership)
target_link_libraries(templatespu ${EXTRA_LIBS})

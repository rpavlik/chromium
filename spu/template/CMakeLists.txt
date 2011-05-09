include_directories(.)
set(SOURCES
	templatespu.c
	templatespu_config.c
	templatespu_init.c
	templatespu.def
)
add_library(templatespu SHARED ${SOURCES})
target_link_libraries(templatespu spuload crutil crmothership)
target_link_libraries(templatespu ${EXTRA_LIBS})
install(TARGETS templatespu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

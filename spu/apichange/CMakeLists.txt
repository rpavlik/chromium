include_directories(.)
set(SOURCES
	apichangespu.c
	apichangespu_config.c
	apichangespu_init.c
	apichangespu.def
)
add_library(apichangespu SHARED ${SOURCES})
target_link_libraries(apichangespu spuload crutil crmothership)
target_link_libraries(apichangespu ${EXTRA_LIBS})
install(TARGETS apichangespu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

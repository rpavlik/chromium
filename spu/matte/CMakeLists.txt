include_directories(.)
set(SOURCES
	mattespu.c
	mattespu_config.c
	mattespu_init.c
)
add_library(mattespu SHARED ${SOURCES})
target_link_libraries(mattespu spuload crutil crmothership)
target_link_libraries(mattespu ${EXTRA_LIBS})
install(TARGETS mattespu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

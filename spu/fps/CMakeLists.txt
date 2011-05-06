include_directories(.)
set(SOURCES
	fpsspu.c
	fpsspu_config.c
	fpsspu_init.c
)
add_library(fpsspu SHARED ${SOURCES})
target_link_libraries(fpsspu spuload crutil crmothership)
target_link_libraries(fpsspu ${EXTRA_LIBS})
install(TARGETS fpsspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

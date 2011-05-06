include_directories(.)
set(SOURCES
	injectorspu.c
	injectorspu_config.c
	injectorspu_init.c
)
add_library(injectorspu SHARED ${SOURCES})
target_link_libraries(injectorspu spuload crutil crmothership injectorspu_crunpacker_copy)
target_link_libraries(injectorspu ${EXTRA_LIBS})

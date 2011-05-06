include_directories(.)
set(SOURCES
	fraglight.c
)
add_executable(fraglight  ${SOURCES})
target_link_libraries(fraglight crutil crfaker crmothership spuload)
target_link_libraries(fraglight ${EXTRA_LIBS})

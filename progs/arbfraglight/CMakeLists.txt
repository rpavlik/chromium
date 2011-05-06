include_directories(.)
set(SOURCES
	arbfraglight.c
)
add_executable(arbfraglight  ${SOURCES})
target_link_libraries(arbfraglight crutil crfaker crmothership spuload)
target_link_libraries(arbfraglight ${EXTRA_LIBS})

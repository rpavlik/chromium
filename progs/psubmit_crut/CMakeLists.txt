include_directories(.)
set(SOURCES
	psubmit_crut.c
)
add_executable(psubmit_crut  ${SOURCES})
target_link_libraries(psubmit_crut crutil crfaker crmothership spuload crutapi crutclientapi)
target_link_libraries(psubmit_crut ${EXTRA_LIBS})
install(TARGETS psubmit_crut
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

include_directories(.)
set(GENERATED
	dlm.def
	../include/cr_dlm.h
	dlm_generated.h
	dlm_generated.c
)
# TODO: generate these files!


set(SOURCES
	dlm.c
	dlm_lists.c
	dlm_pointers.c
	dlm_arrays.c
	dlm_checklist.c
	dlm_error.c
	dlm_calllist.c
	dlm_bbox.c
	${CMAKE_CURRENT_BINARY_DIR}/dlm.def
	${CMAKE_CURRENT_BINARY_DIR}/../include/cr_dlm.h
	${CMAKE_CURRENT_BINARY_DIR}/dlm_generated.h
	${CMAKE_CURRENT_BINARY_DIR}/dlm_generated.c
)
add_library(crdlm SHARED ${SOURCES})
target_link_libraries(crdlm spuload crutil crmothership crstate)
target_link_libraries(crdlm ${EXTRA_LIBS})

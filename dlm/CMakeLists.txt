set(SOURCES
	dlm.c
	dlm_lists.c
	dlm_generated.c
	dlm_pointers.c
	dlm_arrays.c
	dlm_checklist.c
	dlm_error.c
	dlm_calllist.c
	dlm_bbox.c
)
add_library(crdlm SHARED ${SOURCES})
target_link_libraries(crdlm spuload crutil crmothership crstate)

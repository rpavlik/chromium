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
set(GENERATED
	dlm.def
	../include/cr_dlm.h
	dlm_generated.h
	dlm_generated.c
)
# TODO: generate these files!


add_library(crdlm SHARED ${SOURCES})
target_link_libraries(crdlm spuload crutil crmothership crstate)

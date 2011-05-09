include_directories(.)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
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
	${PROJECT_BINARY_DIR}/include/cr_dlm.h
	${CMAKE_CURRENT_BINARY_DIR}/dlm_generated.h
	${CMAKE_CURRENT_BINARY_DIR}/dlm_generated.c
)


add_custom_command(OUTPUT
	${PROJECT_BINARY_DIR}/include/cr_dlm.h
	COMMAND
	${PYTHON_EXECUTABLE} dlm_header.py header > ${PROJECT_BINARY_DIR}/include/cr_dlm.h
	WORKING_DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/dlm_header.py
	${API_FILES})
add_custom_command(OUTPUT
	dlm.def
	COMMAND
	${PYTHON_EXECUTABLE} dlm_header.py defs > ${CMAKE_CURRENT_BINARY_DIR}/dlm.def
	WORKING_DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/dlm_header.py
	${API_FILES})

add_custom_command(OUTPUT
	dlm_generated.h
	COMMAND
	${PYTHON_EXECUTABLE} dlm_generated.py headers > ${CMAKE_CURRENT_BINARY_DIR}/dlm_generated.h
	WORKING_DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/dlm_generated.py
	${API_FILES})
add_custom_command(OUTPUT
	dlm_generated.c
	COMMAND
	${PYTHON_EXECUTABLE} dlm_generated.py source > ${CMAKE_CURRENT_BINARY_DIR}/dlm_generated.c
	WORKING_DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/dlm_generated.py
	${API_FILES})

add_library(crdlm SHARED ${SOURCES})
target_link_libraries(crdlm spuload crutil crmothership crstate)
target_link_libraries(crdlm ${EXTRA_LIBS})
install(TARGETS crdlm
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

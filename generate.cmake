set(APIFILES
	${PROJECT_SOURCE_DIR}/glapi_parser/APIspec.txt
	${PROJECT_SOURCE_DIR}/glapi_parser/apiutil.py)
# Function to automate generation of source files with python scripts
# any extra arguments are assumed to be dependencies.
function(generate_with_python script output)
	if(NOT IS_ABSOLUTE "${output}")
		get_filename_component(output "${CMAKE_CURRENT_BINARY_DIR}/${output}" ABSOLUTE)
	endif()
	add_custom_command(OUTPUT
		${output}
		COMMAND
		${PYTHON_EXECUTABLE} ${script} > ${output}
		WORKING_DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/${script}
		${ARGN})
endfunction()
function(generate_with_python_arg script arg output)
	if(NOT IS_ABSOLUTE "${output}")
		get_filename_component(output "${CMAKE_CURRENT_BINARY_DIR}/${output}" ABSOLUTE)
	endif()
	add_custom_command(OUTPUT
		${output}
		COMMAND
		${PYTHON_EXECUTABLE} ${script} ${arg} > ${output}
		WORKING_DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/${script}
		${ARGN})
endfunction()

function(create_local_apifiles)
	file(RELATIVE_PATH TOP ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR})
	set(LOCAL_APIDIR ${CMAKE_CURRENT_BINARY_DIR}/${TOP}/glapi_parser)
	file(MAKE_DIRECTORY ${LOCAL_APIDIR})

	add_custom_command(OUTPUT
		${PROJECT_SOURCE_DIR}/glapi_parser/APIspec.txt
		COMMAND
		${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/glapi_parser/APIspec.txt ${LOCAL_APIDIR}/APIspec.txt
		DEPENDS
		${PROJECT_SOURCE_DIR}/glapi_parser/APIspec.txt)
	add_custom_command(OUTPUT
		${PROJECT_SOURCE_DIR}/glapi_parser/apiutil.py
		COMMAND
		${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/glapi_parser/apiutil.py ${LOCAL_APIDIR}/apiutil.py
		DEPENDS
		${PROJECT_SOURCE_DIR}/glapi_parser/apiutil.py)
	set(LOCAL_APIFILES "${LOCAL_APIDIR}/APIspec.txt" "${LOCAL_APIDIR}/apiutil.py" PARENT_SCOPE)
endfunction()

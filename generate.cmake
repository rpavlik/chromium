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

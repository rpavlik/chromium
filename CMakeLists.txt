cmake_minimum_required(VERSION 2.8)
project(cr)

find_package(PythonInterp REQUIRED)
find_package(Threads)

set(EXTRA_LIBS)
if(CMAKE_THREAD_LIBS_INIT)
	set(CHROMIUM_THREADSAFE ON)
	list(APPEND EXTRA_LIBS ${CMAKE_THREAD_LIBS_INIT})
	add_definitions(-DCHROMIUM_THREADSAFE=1)
else()
	set(CHROMIUM_THREADSAFE OFF)
endif()

#TODOLINUXSPECIFIC
add_definitions(-DLINUX -DLinux -fPIC)
list(APPEND EXTRA_LIBS dl m)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/include ${PROJECT_BINARY_DIR}/include/state)
include_directories(include ${PROJECT_BINARY_DIR}/include)

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

add_subdirectory(util)
add_subdirectory(mothership)
add_subdirectory(spu_loader)
add_subdirectory(packer)
add_subdirectory(state_tracker)
add_subdirectory(unpacker)
add_subdirectory(dlm)
add_subdirectory(spu)
add_subdirectory(app_faker)
add_subdirectory(opengl_stub)
add_subdirectory(crserverlib)
add_subdirectory(crserver)
add_subdirectory(crutapi)
add_subdirectory(crutclientapi)
add_subdirectory(crutproxy)
add_subdirectory(crutserver)
#add_subdirectory(progs)

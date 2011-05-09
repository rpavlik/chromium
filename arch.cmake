if(MSVC)
	option(USE_FOLDERS "Use project folders? Not compatible with Visual C++ express!" OFF)
else()
	set(USE_FOLDERS ON)
endif()
set_property(GLOBAL PROPERTY USE_FOLDERS ${USE_FOLDERS})

if(WIN32)
	set(ARCH WIN_NT)
	set(X11_PLATFORM OFF)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(MACHTYPE x86_64)
	else()
		set(MACHTYPE i386)
	endif()
	add_definitions(/DWIN32 /DWINDOWS /DGLUT_DISABLE_ATEXIT_HACK)
	if(MSVC)
		add_definitions(/wd4996 /D_CRT_SECURE_NO_DEPRECATE)
	endif()
	list(APPEND EXTRA_LIBS ws2_32)
elseif(APPLE)
	set(X11_PLATFORM OFF)
	add_definitions(-DDARWIN -DGL_GLEXT_LEGACY)
else()
	set(X11_PLATFORM ON)
	set(ARCH ${CMAKE_SYSTEM_NAME})
endif()

if(NOT WIN32)
	execute_process(COMMAND uname -m
		OUTPUT_VARIABLE MACHTYPE
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	if(MACHTYPE MATCHES "i.86")
		set(MACHTYPE "i386")
	endif()
endif()

if("${ARCH}" STREQUAL "Linux")
	add_definitions(-DLINUX -DLinux)
	list(APPEND EXTRA_LIBS dl m)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		# Be able to link in static libs into a shared lib
		add_definitions(-fPIC)
	endif()
endif()


message(STATUS "Build: ${ARCH} on ${MACHTYPE}")


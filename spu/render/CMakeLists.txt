include_directories(.)
set(SOURCES
	renderspu.c
	renderspu_config.c
	renderspu_init.c
)
if(WIN32)
	list(APPEND SOURCES renderspu_wgl.c)
elseif(APPLE)
	list(APPEND SOURCES renderspu_agl.c)
else()
	list(APPEND SOURCES renderspu_glx.c)
endif()
add_library(renderspu SHARED ${SOURCES})
target_link_libraries(renderspu crutil crmothership spuload)
target_link_libraries(renderspu ${EXTRA_LIBS})
install(TARGETS renderspu
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

if(WIN32)
	target_link_libraries(renderspu user32 gdi32)
elseif(APPLE)
	target_link_libraries(renderspu -framework Carbon -framework ApplicationServices)
else()
	#TODO X11 libraries
endif()
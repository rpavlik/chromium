include_directories(.)
set(GENERATED
	packertest0.c
	packertest100.c
	packertest125.c
	packertest150.c
	packertest175.c
	packertest200.c
	packertest225.c
	packertest250.c
	packertest25.c
	packertest275.c
	packertest300.c
	packertest325.c
	packertest350.c
	packertest375.c
	packertest400.c
	packertest425.c
	packertest450.c
	packertest475.c
	packertest500.c
	packertest50.c
	packertest525.c
	packertest550.c
	packertest575.c
	packertest75.c
	packertestBlendFuncSeparateEXT.c
	packertest.c
	packertestColorTable.c
	packertestColorTableEXT.c
	packertestCombinerInputNV.c
	packertest.h
	packertestTexImage1D.c
	packertestTexImage2D.c
	packertestTexImage3D.c
	packertestTexSubImage1D.c
	packertestTexSubImage2D.c
	packertestTexSubImage3D.c
)
# TODO: generate these files!


set(SOURCES
	packertestAttrib.c
	packertestCompressedTexARB.c
	packertestTexFuncs.c
	packertestBeginEnd.c
	packertestBufferARB.c
	packertestCRfuncs.c
	packertestFenceNV.c
	packertestLists.c
	packertestPointParamARB.c
	packertestProgramARB.c
	packertestProgramNV.c
	packertestQueriesARB.c
	packertestglobal.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest0.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest100.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest125.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest150.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest175.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest200.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest225.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest250.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest25.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest275.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest300.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest325.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest350.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest375.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest400.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest425.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest450.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest475.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest500.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest50.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest525.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest550.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest575.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest75.c
	${CMAKE_CURRENT_BINARY_DIR}/packertestBlendFuncSeparateEXT.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest.c
	${CMAKE_CURRENT_BINARY_DIR}/packertestColorTable.c
	${CMAKE_CURRENT_BINARY_DIR}/packertestColorTableEXT.c
	${CMAKE_CURRENT_BINARY_DIR}/packertestCombinerInputNV.c
	${CMAKE_CURRENT_BINARY_DIR}/packertest.h
	${CMAKE_CURRENT_BINARY_DIR}/packertestTexImage1D.c
	${CMAKE_CURRENT_BINARY_DIR}/packertestTexImage2D.c
	${CMAKE_CURRENT_BINARY_DIR}/packertestTexImage3D.c
	${CMAKE_CURRENT_BINARY_DIR}/packertestTexSubImage1D.c
	${CMAKE_CURRENT_BINARY_DIR}/packertestTexSubImage2D.c
	${CMAKE_CURRENT_BINARY_DIR}/packertestTexSubImage3D.c
)

create_local_apifiles()
add_custom_command(OUTPUT
		${GENERATED}
		COMMAND
		${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/packertest.py
		DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/packertest.py
		${LOCAL_APIFILES})

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
add_executable(packertest  ${SOURCES})
target_link_libraries(packertest crutil crfaker crmothership spuload ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_link_libraries(packertest ${EXTRA_LIBS})
install(TARGETS packertest
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)

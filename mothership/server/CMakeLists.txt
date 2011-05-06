include_directories(.)

file(WRITE ${PROJECT_BINARY_DIR}/crconfig.py "import os
arch = \"${ARCH}\"
crdir = \"${CMAKE_INSTALL_PREFIX}\"
crbindir = os.path.join(crdir,'bin/')
crlibdir = os.path.join(crdir,'lib/')")

install(FILES ${PROJECT_BINARY_DIR}/crconfig.py
	DESTINATION share/cr/mothership/server/
	COMPONENT mothership)

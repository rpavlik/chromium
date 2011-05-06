include_directories(.)

file(WRITE ${PROJECT_BINARY_DIR}/crconfig.py "import os
arch = \"${ARCH}\"
crdir = \"${CMAKE_INSTALL_PREFIX}\"
crbindir = os.path.join(crdir,'bin',arch)
crlibdir = os.path.join(crdir,'lib',arch)")

#TODO install to
# /usr/local/share/cr-%{version}/mothership/server/crconfig.py

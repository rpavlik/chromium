TOP = ..

ifneq ($(ARCH), WIN_NT)
ifneq ($(ARCH), WIN_98)
ARCH=$(shell uname | sed -e 's/-//g')
endif
endif

LIBRARY = crfaker
FILES = api_templates init load
LIBRARIES = crutil spuload

ifeq ($(ARCH), WIN_NT)
FILES += windows_exports wgl
ARCH_EXPORTS_PY = windows_exports.py
ARCH_EXPORTS = windows_exports.c
endif

ifeq ($(ARCH), Linux)
FILES += $(ARCH)_exports glx
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.s
endif

ifeq ($(ARCH), IRIX64)
FILES += $(ARCH)_exports glx
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c
endif

ifeq ($(ARCH), IRIX)
FILES += IRIX64_exports glx
ARCH_EXPORTS_PY = IRIX64_exports.py
ARCH_EXPORTS = IRIX64_exports.c
endif

PRECOMP = $(ARCH_EXPORTS) init.c api_templates.h api_templates.c opengl.def
SLOP = $(PRECOMP)

include ${TOP}/cr.mk

ifdef WINDOWS
LIB_DEFS = opengl.def
LDFLAGS += user32.lib ws2_32.lib
endif

opengl.def: stub_common.py defs.py
	@$(ECHO) "Creating the defs file..."
	@$(PYTHON) defs.py > opengl.def

api_templates.h: stub_common.py api_templates_header.py
	@$(ECHO) "Creating the OpenGL indirection header"
	@$(PYTHON) api_templates_header.py > api_templates.h

api_templates.c: stub_common.py api_templates.py
	@$(ECHO) "Creating the OpenGL indirection file"
	@$(PYTHON) api_templates.py > api_templates.c

init.c: stub_common.py init.py
	@$(ECHO) "Creating the SPU Initializer"
	@$(PYTHON) init.py > init.c

$(ARCH_EXPORTS): stub_common.py $(ARCH_EXPORTS_PY)
	@$(ECHO) "Creating the OpenGL DLL export functions $(ARCH)"
	@$(PYTHON) $(ARCH_EXPORTS_PY) > $(ARCH_EXPORTS)

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = crfaker
FILES = getprocaddress load stub context dummyfuncs
LIBRARIES = crmothership crutil spuload

ifdef THREADSAFE
FILES += tsfuncs
endif

ifeq ($(ARCH), WIN_NT)
FILES += windows_exports wgl
ARCH_EXPORTS_PY = windows_exports.py
ARCH_EXPORTS = windows_exports.c
endif

ifeq ($(ARCH), Linux)
LDFLAGS += -lX11
ifeq ($(MACHTYPE), i386)
FILES += $(ARCH)_$(MACHTYPE)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_$(MACHTYPE)_exports.py
ARCH_EXPORTS = $(ARCH)_$(MACHTYPE)_exports.s
else
FILES += $(ARCH)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif
endif

ifeq ($(ARCH), SunOS)
LDFLAGS += -lX11
FILES += $(ARCH)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c
endif

ifeq ($(ARCH), FreeBSD)
LDFLAGS += -lX11
FILES += $(ARCH)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif

ifeq ($(ARCH), AIX)
OPENGL = 1
FILES += $(ARCH)_exports glx xfont AIX_edgeflagpointer
ARCH_EXPORTS_PY = IRIX64_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif

ifeq ($(ARCH), Darwin)
LDFLAGS += -lX11
FILES += $(ARCH)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif

ifeq ($(ARCH), IRIX64)
FILES += $(ARCH)_exports glx IRIX_edgeflagpointer xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif

ifeq ($(ARCH), IRIX)
FILES += IRIX64_exports glx IRIX_edgeflagpointer xfont
ARCH_EXPORTS_PY = IRIX64_exports.py
ARCH_EXPORTS = IRIX64_exports.c 
endif

ifeq ($(ARCH), OSF1)
LDFLAGS += -lX11
FILES += $(ARCH)_exports glx xfont OSF1_edgeflagpointer
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif

PRECOMP = $(ARCH_EXPORTS) getprocaddress.c opengl.def
SLOP = $(PRECOMP)

LIB_DEFS = opengl.def

include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib ws2_32.lib gdi32.lib
endif

opengl.def: stub_common.py defs.py
	@$(ECHO) "Creating the defs file..."
	@$(PYTHON) defs.py > opengl.def

getprocaddress.c: getprocaddress.py getprocaddress_special alias_exports.py
	@$(ECHO) "Creating crGetProcAddress"
	@$(PYTHON) getprocaddress.py > getprocaddress.c

$(ARCH_EXPORTS): stub_common.py $(ARCH_EXPORTS_PY) alias_exports.py
	@$(ECHO) "Creating the OpenGL DLL export functions (for $(ARCH))"
	@$(PYTHON) $(ARCH_EXPORTS_PY) > $(ARCH_EXPORTS)

tsfuncs.c: tsfuncs.py ../glapi_parser/gl_header.parsed
	@$(ECHO) "Building thread-safe dispatch code"
	@$(PYTHON) tsfuncs.py > tsfuncs.c

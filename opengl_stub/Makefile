# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = crfaker
FILES = getprocaddress load stub context NULLfuncs
LIBRARIES = crmothership crutil spuload

ifeq ($(THREADSAFE), 1)
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
ifeq ($(MACHTYPE),x86_64)
FILES += $(ARCH)_$(MACHTYPE)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_$(MACHTYPE)_exports.py
ARCH_EXPORTS = $(ARCH)_$(MACHTYPE)_exports.c
else
FILES += $(ARCH)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif
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
FILES += $(ARCH)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif

ifeq ($(ARCH), Darwin)
LDFLAGS += -L/System/Library/Frameworks/OpenGL.framework/Libraries
LDFLAGS += -framework Carbon -compatibility_version 1.0.0 -current_version 1.0.0 -sub_library libGLU -lGLU
FILES += $(ARCH)_exports cgl
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif

ifeq ($(ARCH), IRIX64)
FILES += $(ARCH)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif

ifeq ($(ARCH), IRIX)
FILES += IRIX64_exports glx xfont
ARCH_EXPORTS_PY = IRIX64_exports.py
ARCH_EXPORTS = IRIX64_exports.c 
endif

ifeq ($(ARCH), OSF1)
LDFLAGS += -lX11
FILES += $(ARCH)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif

PRECOMP = $(ARCH_EXPORTS) getprocaddress.c opengl.def NULLfuncs.c tsfuncs.c
SLOP = $(PRECOMP)

LIB_DEFS = opengl.def

include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib ws2_32.lib gdi32.lib
endif

APIFILES = ../glapi_parser/apiutil.py ../glapi_parser/APIspec.txt

opengl.def: defs.py $(APIFILES)
	@$(ECHO) "Creating the defs file..."
	@$(PYTHON) defs.py > opengl.def
ifeq ($(ARCH), WIN_NT)
getprocaddress.c: windows_getprocaddress.py $(APIFILES)
	@$(ECHO) "Creating crGetProcAddress"
	@$(PYTHON) windows_getprocaddress.py > getprocaddress.c
else
getprocaddress.c: getprocaddress.py $(APIFILES)
	@$(ECHO) "Creating crGetProcAddress"
	@$(PYTHON) getprocaddress.py > getprocaddress.c
endif

$(ARCH_EXPORTS): $(ARCH_EXPORTS_PY) $(APIFILES) entrypoints.py
	@$(ECHO) "Creating the OpenGL DLL export functions (for $(ARCH))"
	@$(PYTHON) $(ARCH_EXPORTS_PY) > $(ARCH_EXPORTS)

NULLfuncs.c: NULLfuncs.py $(APIFILES)
	@$(ECHO) "Creating the NULL dispatch table functions"
	@$(PYTHON) NULLfuncs.py > NULLfuncs.c

tsfuncs.c: tsfuncs.py $(APIFILES)
	@$(ECHO) "Building thread-safe dispatch code"
	@$(PYTHON) tsfuncs.py > tsfuncs.c

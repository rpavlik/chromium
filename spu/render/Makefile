# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..
SPU = 1

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = renderspu
FILES = renderspu \
	renderspu_config \
	renderspu_init

ifdef WINDOWS
FILES += renderspu_wgl
else
ifdef DARWIN
FILES += renderspu_agl
else
FILES += renderspu_glx
endif
endif

LIBRARIES += crutil crmothership spuload

SLOP = $(PRECOMP)

LIB_DEFS += render.def
include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib gdi32.lib
else
ifdef DARWIN
LDFLAGS += -framework Carbon -framework ApplicationServices
else
LDFLAGS += -lXmu -lX11
endif
endif

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = renderspu
FILES = renderspu \
	renderspu_config \
	renderspu_init

ifeq ($(ARCH), WIN_NT)
FILES += renderspu_wgl
else
FILES += renderspu_glx
endif

LIBRARIES = spuload crmothership crutil

SLOP = $(PRECOMP)

LIB_DEFS += render.def
include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib gdi32.liB
else
LDFLAGS += -lXmu -lX11
endif

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = renderspu
FILES = renderspu renderspu_config renderspu_init renderspu_swap renderspu_window

LIBRARIES = spuload crmothership crutil

PRECOMP = renderspu.c
SLOP = $(PRECOMP)

LIB_DEFS += render.def
include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib gdi32.lib
else
LDFLAGS += -lXmu -lX11
endif

renderspu.c: render.py render_special render_nop_special render_extensions_special
	@$(ECHO) Building the Render SPU
	@$(PYTHON) render.py > renderspu.c

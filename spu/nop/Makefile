# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = nopspu
FILES = nopspu nopspu_init

ifdef WINDOWS
TRACKS_STATE = 1
LIBRARIES = crutil spuload
else
LIBRARIES = crutil spuload crstate
endif


ifdef BINUTIL_LINK_HACK
TRACKS_STATE = 1
LIBRARIES -= crstate
endif


PRECOMP = nopspu.c
SLOP = $(PRECOMP)

LIB_DEFS += nop.def

include ${TOP}/cr.mk

nopspu.c: nop.py nop_state_special
	@$(ECHO) Building the NOP SPU
	@$(PYTHON) $< > $@

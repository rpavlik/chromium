# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = nopspu
FILES = nopspu nopspu_init

LIBRARIES = crutil
TRACKS_STATE = 1

PRECOMP = nopspu.c
SLOP = $(PRECOMP)

LIB_DEFS += nop.def
include ${TOP}/cr.mk

nopspu.c: nop.py nop_state_special
	@$(ECHO) Building the NOP SPU
	@$(PYTHON) $< > $@

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
include ${TOP}/arch.mk

SHARED = 1
LIBRARY = nopspu
FILES = nopspu nopspu_init nopspu_config

ifdef WINDOWS
TRACKS_STATE = 1
LIBRARIES = crmothership crutil spuload
else
LIBRARIES = crmothership crutil spuload crstate
endif


ifdef BINUTIL_LINK_HACK
TRACKS_STATE = 1
LIBRARIES -= crstate
endif


PRECOMP = nopspu.c
SLOP = $(PRECOMP)

LIB_DEFS += nop.def

include ${TOP}/cr.mk

nopspu.c: nop.py nop_special ../../glapi_parser/APIspec.txt
	@$(ECHO) Building the NOP SPU
	@$(PYTHON) $< > $@

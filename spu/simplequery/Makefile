# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU=1
include ${TOP}/arch.mk


SHARED = 1
LIBRARY = simplequeryspu
FILES = simplequeryspu \
	simplequeryspu_config \
	simplequeryspu_init

ifdef WINDOWS
TRACKS_STATE = 1
LIBRARIES = spuload crutil crmothership
else
LIBRARIES = spuload crutil crmothership crstate
endif

ifdef BINUTIL_LINK_HACK
TRACKS_STATE = 1
LIBRARIES -= crstate
endif

PRECOMP = simplequeryspu.c
SLOP = $(PRECOMP)

LIB_DEFS += simplequeryspu.def

include ${TOP}/cr.mk

simplequeryspu.c: simplequery.py ../../glapi_parser/APIspec.txt
	@$(ECHO) Building the Simple Query SPU
	@$(PYTHON) $< > $@

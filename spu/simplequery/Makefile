# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

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

PRECOMP = simplequeryspu.c
SLOP = $(PRECOMP)

LIB_DEFS += simplequeryspu.def

include ${TOP}/cr.mk

simplequeryspu.c: simplequery.py simplequery_special
	@$(ECHO) Building the Simple Query SPU
	@$(PYTHON) $< > $@

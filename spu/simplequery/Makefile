# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = simplequeryspu
FILES = simplequeryspu \
	simplequeryspu_config \
	simplequeryspu_init

LIBRARIES = spuload crutil crmothership
TRACKS_STATE = 1

PRECOMP = simplequeryspu.c
SLOP = $(PRECOMP)

LIB_DEFS += simplequeryspu.def
include ${TOP}/cr.mk

simplequeryspu.c: simplequery.py simplequery_special
	@$(ECHO) Building the Simple Query SPU
	@$(PYTHON) $< > $@

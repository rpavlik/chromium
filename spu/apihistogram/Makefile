# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
SHARED = 1
LIBRARY = apihistogramspu
FILES = apihistogramspu \
	apihistogramspu_config \
	apihistogramspu_init

LIBRARIES = spuload crutil crmothership

PRECOMP = apihistogramspu.c
SLOP = $(PRECOMP)

LIB_DEFS += apihistogramspu.def
include ${TOP}/cr.mk

apihistogramspu.c: histogram.py
	@$(ECHO) "Making histogram SPU"
	@$(PYTHON) histogram.py > $@

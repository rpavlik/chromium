# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = passthroughspu
FILES = passthroughspu passthroughspu_init

LIBRARIES = crutil

PRECOMP = passthroughspu.c
SLOP = $(PRECOMP)

LIB_DEFS += passthrough.def
include ${TOP}/cr.mk

passthroughspu.c: passthrough.py
	@$(ECHO) Building the Pass Through SPU
	@$(PYTHON) $< > $@

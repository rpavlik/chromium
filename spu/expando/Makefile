# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = expandospu
FILES = expandospu \
	expandospu_config \
	expandospu_init

LIBRARIES = spuload crutil crmothership crdlm

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += expandospu.def
include ${TOP}/cr.mk

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU=1
SHARED = 1
LIBRARY = apichangespu
FILES = apichangespu \
	apichangespu_config \
	apichangespu_init

LIBRARIES = spuload crutil crmothership

SLOP = $(PRECOMP)

LIB_DEFS += apichangespu.def
include ${TOP}/cr.mk

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../../..

LIBRARY = crextensionscommon
FILES = logo
SHARED = 1
OPENGL = 1

LIBRARIES = crutil
ifeq ($(ARCH),AIX)
LIBRARIES += GL
endif

LIB_DEFS = common.def
include ${TOP}/cr.mk

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../../..

include ${TOP}/arch.mk

LIBRARY = crextensionscommon
FILES = logo util
SHARED = 1
OPENGL = 1
GLUT = 1

LIBRARIES = crutil
ifeq ($(ARCH),AIX)
LIBRARIES += GL
endif

LIB_DEFS = common.def
include ${TOP}/cr.mk

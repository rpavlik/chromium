# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../../..

include ${TOP}/arch.mk

PROGRAM = EXT_secondary_color
OPENGL = 1
GLUT = 1

FILES = main 
LIBRARIES = crutil crextensionscommon

ifeq ($(ARCH), SunOS)
LDFLAGS += -lcrfaker 
endif

include ${TOP}/cr.mk

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

PROGRAM = threadtest
FILES = threadtest
MATH = 1

LIBRARIES = crutil crfaker crmothership spuload

ifeq ($(ARCH), SunOS)
LDFLAGS += -lX11
endif

include ${TOP}/cr.mk

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..
ifneq ($(ARCH), WIN_NT)
ifneq ($(ARCH), WIN_98)
ARCH=$(shell uname | sed -e 's/-//g')
endif
endif

ifeq ($(ARCH),CYGWIN_NT5.0)
ARCH=WIN_NT
endif

ifeq ($(ARCH),CYGWIN_NT5.1)
ARCH=WIN_NT
endif

SHARED = 1
LIBRARY = renderspu
FILES = renderspu \
	renderspu_config \
	renderspu_init \
	renderspu_swap

ifeq ($(ARCH), WIN_NT)
FILES += renderspu_wgl
else
FILES += renderspu_glx
endif

LIBRARIES = spuload crmothership crutil

SLOP = $(PRECOMP)

LIB_DEFS += render.def
include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib gdi32.liB
else
LDFLAGS += -lXmu -lX11
endif

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = hiddenlinespu
FILES = hiddenlinespu \
	hiddenlinespu_buffer \
	hiddenlinespu_clear \
	hiddenlinespu_clearcolor \
	hiddenlinespu_config \
	hiddenlinespu_context \
	hiddenlinespu_depthmask \
	hiddenlinespu_disable \
	hiddenlinespu_init \
	hiddenlinespu_pixel \
	hiddenlinespu_swap

ifdef WINDOWS
TRACKS_STATE=1
LIBRARIES = spuload crutil crmothership crpacker
else
LIBRARIES = spuload crutil crmothership crpacker crstate
endif

ifdef BINUTIL_LINK_HACK
TRACKS_STATE = 1
LIBRARIES -= crstate
endif

PACKS = 1
UNPACKS = 1

PRECOMP = hiddenlinespu.c hiddenlinespu_pixel.c
SLOP = $(PRECOMP)

LIB_DEFS += hiddenlinespu.def

include ${TOP}/cr.mk

ifeq ($(ARCH), WIN_NT)
LDFLAGS += user32.lib
endif

hiddenlinespu.c: hiddenline.py hiddenline_special hiddenline_pixel_special hiddenline_ignore_special
	@$(ECHO) Building the Hidden Line SPU
	@$(PYTHON) hiddenline.py > $@

hiddenlinespu_pixel.c: hiddenlinespu_pixel.py hiddenline_pixel_special
	@$(ECHO) Building the Pack SPU Pixel functions
	@$(PYTHON) hiddenlinespu_pixel.py > $@

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU=1
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

PRECOMP = hiddenlinespu.c hiddenlinespu_pixel.c hiddenlinespu_proto.h
SLOP = $(PRECOMP)

LIB_DEFS += hiddenlinespu.def

include ${TOP}/cr.mk

ifeq ($(ARCH), WIN_NT)
LDFLAGS += user32.lib
endif

APIFILES = ../../glapi_parser/apiutil.py ../../glapi_parser/APIspec.txt

hiddenlinespu.c: hiddenline.py hiddenline_special hiddenline_pixel_special $(APIFILES) hiddenlinespu_proto.h
	@$(ECHO) Building the Hidden Line SPU
	@$(PYTHON) hiddenline.py > $@

hiddenlinespu_pixel.c: hiddenlinespu_pixel.py hiddenline_pixel_special $(APIFILES)
	@$(ECHO) Building the Pack SPU Pixel functions
	@$(PYTHON) hiddenlinespu_pixel.py > $@

hiddenlinespu_proto.h: hiddenlinespu_proto.py hiddenline_special hiddenline_pixel_special $(APIFILES)
	@$(ECHO) Building the Hidden Line SPU prototypes header
	@$(PYTHON) hiddenlinespu_proto.py > $@

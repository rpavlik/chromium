# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = packspu
FILES = packspu \
	packspu_client \
	packspu_config \
	packspu_get \
	packspu_getstring \
	packspu_flush \
	packspu_init \
	packspu_net \
	packspu_pixel

LIBRARIES = crutil crmothership crpacker
TRACKS_STATE = 1

PRECOMP = packspu.c packspu_get.c packspu_flush.c packspu_pixel.c 
SLOP = $(PRECOMP)

LIB_DEFS += pack.def
include ${TOP}/cr.mk

packspu.c: pack.py packspu_pixel_special packspu_special packspu_unimplemented_special
	@$(ECHO) Building the Pack SPU
	@$(PYTHON) pack.py > $@
	
packspu_get.c: packspu_get.py packspu_special ../../crserver/get_sizes.py
	@$(ECHO) Building the Pack SPU Get functions
	@$(PYTHON) packspu_get.py > $@	

packspu_flush.c: packspu_flush.py packspu_flush_special
	@$(ECHO) Building the Pack SPU Auto-flushing functions
	@$(PYTHON) packspu_flush.py > $@

packspu_pixel.c: packspu_pixel.py packspu_pixel_special
	@$(ECHO) Building the Pack SPU Pixel functions
	@$(PYTHON) packspu_pixel.py > $@

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = tilesortspu
FILES = tilesortspu \
	tilesortspu_bcast \
	tilesortspu_beginend \
	tilesortspu_bucket \
	tilesortspu_client \
	tilesortspu_config \
	tilesortspu_context \
	tilesortspu_copytex \
	tilesortspu_diffapi \
	tilesortspu_eval \
	tilesortspu_flush \
	tilesortspu_get \
	tilesortspu_init \
	tilesortspu_lists \
	tilesortspu_material \
	tilesortspu_misc \
	tilesortspu_net \
	tilesortspu_pinch_gen \
	tilesortspu_pixels \
	tilesortspu_rect \
	tilesortspu_swap \
	tilesortspu_viewport

LIBRARIES = crutil crmothership spuload
TRACKS_STATE = 1
PACKS = 1

PRECOMP = tilesortspu.c tilesortspu_lists.c tilesortspu_diffapi.c tilesortspu_pinch_gen.c
SLOP = $(PRECOMP)

LIB_DEFS += tilesort.def
include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib gdi32.lib
else
LDFLAGS += -lXmu -lX11
endif

tilesortspu.c: tilesort.py tilesort_special tilesort_state_special tilesort_unimplemented_special
	@$(ECHO) Building the TileSort SPU
	@$(PYTHON) tilesort.py > $@

tilesortspu_diffapi.c: diffapi.py
	@$(ECHO) Building the TileSort Differencing API
	@$(PYTHON) diffapi.py > $@

tilesortspu_lists.c: tilesort_lists.py tilesort_list_special tilesort_unimplemented_special tilesort_special tilesort_bbox_special tilesort_state_special
	@$(ECHO) Building the TileSort Display List Manager
	@$(PYTHON) tilesort_lists.py > $@

tilesortspu_pinch_gen.c: tilesortspu_pinch.c ../state_tracker/convert.py length_table.py pinch_convert.py
	@$(ECHO) Munging the pincher... yuck
	@$(PYTHON) ../state_tracker/convert.py > $@
	@$(PYTHON) pinch_convert.py >> $@
	@$(PYTHON) length_table.py >> $@
	@$(CAT) tilesortspu_pinch.c >> $@

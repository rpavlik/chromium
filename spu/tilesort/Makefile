# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = tilesortspu
FILES = tilesortspu \
	tilesortspu_bcast \
	tilesortspu_beginend \
	tilesortspu_bucket \
	tilesortspu_bufferobject \
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
	tilesortspu_lists_gen \
	tilesortspu_misc \
	tilesortspu_net \
	tilesortspu_occlude \
	tilesortspu_pinch_gen \
	tilesortspu_pixels \
	tilesortspu_rasterpos \
	tilesortspu_rect \
	tilesortspu_swap \
	tilesortspu_tiles \
	tilesortspu_unimplemented \
	tilesortspu_viewport \
	tilesortspu_window

ifdef WINDOWS
TRACKS_STATE = 1
LIBRARIES = crutil crmothership crdlm spuload
else
LIBRARIES = crutil crmothership crdlm spuload crstate
endif

ifdef BINUTIL_LINK_HACK
TRACKS_STATE = 1
LIBRARIES -= crstate
endif

PACKS = 1

PRECOMP = tilesortspu.c tilesortspu_lists_gen.c tilesortspu_diffapi.c tilesortspu_pinch_gen.c tilesortspu_unimplemented.c tilesortspu_proto.h
SLOP = $(PRECOMP)

LIB_DEFS += tilesort.def

include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib gdi32.lib
else
LDFLAGS += -lXmu -lX11
ifeq ($(USE_DMX), 1)
LDFLAGS += -ldmx
endif
endif

APIFILES = ../../glapi_parser/apiutil.py ../../glapi_parser/APIspec.txt

tilesortspu.c: tilesort.py tilesort_special tilesort_state_special tilesort_unimplemented_special $(APIFILES) tilesortspu_proto.h
	@$(ECHO) Building the TileSort SPU
	@$(PYTHON) tilesort.py > $@

tilesortspu_unimplemented.c: tilesort_unimplemented.py tilesort_unimplemented_special $(APIFILES)
	@$(ECHO) Building the TileSort Unimplemented Functions
	@$(PYTHON) tilesort_unimplemented.py > $@

tilesortspu_diffapi.c: diffapi.py $(APIFILES)
	@$(ECHO) Building the TileSort Differencing API
	@$(PYTHON) diffapi.py > $@

tilesortspu_lists_gen.c: tilesort_lists.py tilesort_unimplemented_special tilesort_special tilesort_bbox_special tilesort_state_special $(APIFILES)
	@$(ECHO) Building the TileSort Display List Manager
	@$(PYTHON) tilesort_lists.py > $@

tilesortspu_pinch_gen.c: tilesortspu_pinch.c ../../state_tracker/convert.py length_table.py pinch_convert.py $(APIFILES)
	@$(ECHO) Munging the pincher... yuck
	@$(PYTHON) ../../state_tracker/convert.py > $@
	@$(PYTHON) pinch_convert.py >> $@
	@$(PYTHON) length_table.py >> $@
	@$(CAT) tilesortspu_pinch.c >> $@

tilesortspu_proto.h: tilesortspu_proto.py tilesort_special tilesort_state_special tilesort_unimplemented_special $(APIFILES)
	@$(ECHO) Building the TileSort SPU prototypes header
	@$(PYTHON) tilesortspu_proto.py > $@

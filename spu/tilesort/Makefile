# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
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
	tilesortspu_dlpack_gen \
	tilesortspu_dlpacker_gen \
	tilesortspu_dlstate_gen \
	tilesortspu_eval \
	tilesortspu_flush \
	tilesortspu_gen \
	tilesortspu_get \
	tilesortspu_init \
	tilesortspu_lists \
	tilesortspu_misc \
	tilesortspu_material \
	tilesortspu_net \
	tilesortspu_occlude \
	tilesortspu_pinch_gen \
	tilesortspu_pixels \
	tilesortspu_rect \
	tilesortspu_stereo \
	tilesortspu_swap \
	tilesortspu_tiles \
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

PRECOMP = tilesortspu_gen.h \
	tilesortspu_gen.c \
	tilesortspu.c \
	tilesortspu_diffapi.c \
	tilesortspu_pinch_gen.c \
	tilesortspu_dlpack_gen.c \
	tilesortspu_dlpack_gen.h \
	tilesortspu_dlstate_gen.c \
	tilesortspu_dlstate_gen.h \
	tilesortspu_dlpacker_gen.c

SLOP = $(PRECOMP)

LIB_DEFS += tilesort.def

include ${TOP}/cr.mk

ifdef WINDOWS
    LDFLAGS += user32.lib gdi32.lib
else
ifdef DARWIN
    LDFLAGS += -framework Carbon
else
    LDFLAGS += -lXmu -lX11
    ifeq ($(USE_DMX), 1)
        ifdef DMX_LIBDIR
            LDFLAGS += -L${DMX_LIBDIR}
        endif
        LDFLAGS += -ldmx
        ifdef DMX_INCDIR
            CFLAGS += -I${DMX_INCDIR}
        endif
    endif
endif
endif

APIFILES = ../../glapi_parser/apiutil.py ../../glapi_parser/APIspec.txt

tilesortspu.c: generator.py tilesort_special tilesort_unimplemented_special $(APIFILES) tilesortspu_gen.h
	@$(ECHO) Building the TileSort SPU
	@$(PYTHON) generator.py table > $@

tilesortspu_gen.h: generator.py tilesort_special tilesort_unimplemented_special $(APIFILES)
	@$(ECHO) Building the TileSort Generated Prototypes
	@$(PYTHON) generator.py header > $@

tilesortspu_gen.c: generator.py tilesort_special tilesort_unimplemented_special $(APIFILES) tilesortspu_gen.h
	@$(ECHO) Building the TileSort Generated Functions
	@$(PYTHON) generator.py source > $@

tilesortspu_dlpack_gen.c: dlpack_generator.py tilesort_dlpack_ignore_special tilesortspu_dlpack_gen.h $(APIFILES)
	@$(ECHO) Building the TileSort DL Packer Generated Functions
	@$(PYTHON) dlpack_generator.py source > $@

tilesortspu_dlpacker_gen.c: dlpack_generator.py tilesort_dlpack_ignore_special tilesortspu_dlpack_gen.h $(APIFILES)
	@$(ECHO) Building the TileSort DL Packer Loader
	@$(PYTHON) dlpack_generator.py table > $@

tilesortspu_dlpack_gen.h: dlpack_generator.py tilesort_dlpack_ignore_special $(APIFILES)
	@$(ECHO) Building the TileSort DL Packer Header
	@$(PYTHON) dlpack_generator.py header > $@

tilesortspu_dlstate_gen.c: dlstate_generator.py tilesortspu_dlstate_gen.h $(APIFILES)
	@$(ECHO) Building the TileSort DL State Loader
	@$(PYTHON) dlstate_generator.py table > $@

tilesortspu_dlstate_gen.h: dlstate_generator.py $(APIFILES)
	@$(ECHO) Building the TileSort DL State Header
	@$(PYTHON) dlstate_generator.py header > $@


tilesortspu_diffapi.c: diffapi.py $(APIFILES)
	@$(ECHO) Building the TileSort Differencing API
	@$(PYTHON) diffapi.py > $@

tilesortspu_pinch_gen.c: tilesortspu_pinch.c ../../state_tracker/convert.py length_table.py pinch_convert.py $(APIFILES)
	@$(ECHO) Munging the pincher... yuck
	@$(PYTHON) ../../state_tracker/convert.py > $@
	@$(PYTHON) pinch_convert.py >> $@
	@$(PYTHON) length_table.py >> $@
	@$(CAT) tilesortspu_pinch.c >> $@

TOP = ../..

SHARED = 1
LIBRARY = tilesortspu
FILES = tilesortspu \
	tilesortspu_bcast \
	tilesortspu_beginend \
	tilesortspu_bucket \
	tilesortspu_config \
	tilesortspu_diffapi \
	tilesortspu_flush \
	tilesortspu_init \
	tilesortspu_misc \
	tilesortspu_net \
	tilesortspu_swap \
	tilesortspu_viewport

LIBRARIES = crutil crmothership crstate crpacker

PRECOMP = tilesortspu.c tilesortspu_diffapi.c
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

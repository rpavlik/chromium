TOP = ../..

SHARED = 1
LIBRARY = tilesortspu
FILES = tilesortspu tilesortspu_bcast tilesortspu_beginend \
	tilesortspu_config tilesortspu_init \
	tilesortspu_misc tilesortspu_net tilesortspu_swap

LIBRARIES = crutil crmothership crstate crpacker

PRECOMP = tilesortspu.c
SLOP = $(PRECOMP)

LIB_DEFS += tilesort.def
include ${TOP}/cr.mk

tilesortspu.c: tilesort.py tilesort_special tilesort_state_special tilesort_unimplemented_special
	@$(ECHO) Building the TileSort SPU
	@$(PYTHON) tilesort.py > $@

TOP = ../..

SHARED = 1
LIBRARY = printspu
FILES = printspu \
		printspu_config \
		printspu_enums \
		printspu_init \
		printspu_matrices

LIBRARIES = crutil crmothership spuload

PRECOMP = printspu.c
SLOP = $(PRECOMP)

LIB_DEFS += print.def
include ${TOP}/cr.mk

printspu.c: print.py printspu_unimplemented_special printspu_special
	@$(ECHO) Building the Print SPU
	@$(PYTHON) print.py > $@

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SHARED = 1
LIBRARY = printspu
FILES = printspu \
		printspu_config \
		printspu_enums \
		printspu_get \
		printspu_init \
		printspu_lighting \
		printspu_matrices \
		printspu_misc

LIBRARIES = crutil crmothership spuload

PRECOMP = printspu.c
SLOP = $(PRECOMP)

SPU_COPIES = COPYprintspu

LIB_DEFS += print.def
include ${TOP}/cr.mk

printspu.c: print.py printspu_unimplemented_special printspu_special
	@$(ECHO) Building the Print SPU
	@$(PYTHON) print.py > $@

TOP = ../..

SHARED = 1
LIBRARY = nopspu
FILES = nopspu nopspu_init

LIBRARIES = crutil

PRECOMP = nopspu.c
SLOP = $(PRECOMP)

LIB_DEFS += nop.def
include ${TOP}/cr.mk

nopspu.c: nop.py
	@$(ECHO) Building the NOP SPU
	@$(PYTHON) $< > $@

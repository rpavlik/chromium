TOP = ../..

SHARED = 1
LIBRARY = packspu
FILES = packspu packspu_config packspu_init packspu_net

LIBRARIES = crutil crmothership crstate crpacker

PRECOMP = packspu.c
SLOP = $(PRECOMP)

LIB_DEFS += pack.def
include ${TOP}/cr.mk

packspu.c: pack.py
	@$(ECHO) Building the Pack SPU
	@$(PYTHON) pack.py > packspu.c

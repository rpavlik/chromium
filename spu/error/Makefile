TOP = ../..

SHARED = 1
LIBRARY = errorspu
FILES = errorspu errorspu_init

LIBRARIES = crutil

PRECOMP = errorspu.c
SLOP = $(PRECOMP)

LIB_DEFS += error.def
include ${TOP}/cr.mk

errorspu.c: error.py
	@$(ECHO) Building the Error SPU
	@$(PYTHON) error.py > $@

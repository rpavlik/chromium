TOP = ../..

LIBRARY = errorspu
FILES = errorspu errorspu_init

LIBRARIES = crutil

PRECOMP = errorspu.c
SLOP = $(PRECOMP)

include ${TOP}/cr.mk

errorspu.c: error.py
	@$(ECHO) Building the Error SPU
	@$(PYTHON) error.py > errorspu.c

ifeq ($(ARCH), WIN_NT)
LIB_DEFS += error.def
endif

TOP = ../..

LIBRARY = errorspu
FILES = error init

LIBRARIES = crutil

PRECOMP = error.c
SLOP = $(PRECOMP)

include ${TOP}/cr.mk

error.c: error.py
	@$(ECHO) Building the Error SPU
	@$(PYTHON) error.py > error.c

ifeq ($(ARCH), WIN_NT)
LIB_DEFS += error.def
endif

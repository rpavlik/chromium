TOP = ../..

LIBRARY = renderspu
FILES = renderspu renderspu_init

LIBRARIES = crutil

PRECOMP = renderspu.c
SLOP = $(PRECOMP)

include ${TOP}/cr.mk

renderspu.c: render.py render_special
	@$(ECHO) Building the Render SPU
	@$(PYTHON) render.py > renderspu.c

ifeq ($(ARCH), WIN_NT)
LIB_DEFS += render.def
endif

TOP = ../..

LIBRARY = passthroughspu
FILES = passthroughspu passthroughspu_init

LIBRARIES = crutil

PRECOMP = passthroughspu.c
SLOP = $(PRECOMP)

LIB_DEFS += passthrough.def
include ${TOP}/cr.mk

passthroughspu.c: passthrough.py
	@$(ECHO) Building the Pass Through SPU
	@$(PYTHON) $< > $@

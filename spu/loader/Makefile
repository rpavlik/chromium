TOP = ../../
LIBRARY = spuload
LIBRARIES = crutil

FILES = load dispatch

PRECOMP = $(TOP)/include/spu_dispatch_table.h dispatch.c
SLOP = $(PRECOMP)

LIB_DEFS += loader.def
include ${TOP}/cr.mk

$(TOP)/include/spu_dispatch_table.h: dispatchheader.py
	@$(ECHO) Making the SPU dispatch header
	@$(PYTHON) dispatchheader.py > $(TOP)/include/spu_dispatch_table.h

dispatch.c: dispatch.py
	@$(ECHO) Making the SPU dispatch builder
	@$(PYTHON) dispatch.py > dispatch.c

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../../
SHARED = 1
LIBRARY = spuload
LIBRARIES = crutil crmothership

FILES = spuchange spucopy spuinit spuload dispatch limits

PRECOMP = $(TOP)/include/spu_dispatch_table.h dispatch.c spucopy.c spuchange.c
SLOP = $(PRECOMP)

LIB_DEFS += loader.def
include ${TOP}/cr.mk

$(TOP)/include/spu_dispatch_table.h: dispatchheader.py
	@$(ECHO) Making the SPU dispatch header
	@$(PYTHON) dispatchheader.py > $(TOP)/include/spu_dispatch_table.h

dispatch.c: dispatch.py
	@$(ECHO) Making the SPU dispatch builder
	@$(PYTHON) dispatch.py > dispatch.c

spucopy.c: spucopy.py
	@$(ECHO) Making the SPU copier
	@$(PYTHON) spucopy.py > spucopy.c

spuchange.c: spuchange.py
	@$(ECHO) Making the SPU interface changer
	@$(PYTHON) spuchange.py > spuchange.c

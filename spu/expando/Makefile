# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
SHARED = 1
LIBRARY = expandospu
FILES = expandospu \
	expandospu_config \
	expandospu_init \
	expando

LIBRARIES = spuload crutil crmothership crdlm crstate

PRECOMP = expando.c 
SLOP = $(PRECOMP)

LIB_DEFS += expandospu.def
include ${TOP}/cr.mk
APIFILES = ../../glapi_parser/apiutil.py ../../glapi_parser/APIspec.txt

expando.c: expando.py expando_special $(APIFILES)
	@$(ECHO) Generating Expando SPU functions
	@$(PYTHON) expando.py > $@

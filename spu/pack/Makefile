# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

SPU = 1
include ${TOP}/arch.mk

SHARED = 1
LIBRARY = packspu
FILES = packspu \
	packspu_beginend \
	packspu_bufferobject \
	packspu_client \
	packspu_config \
	packspu_context \
	packspu_get \
	packspu_getstring \
	packspu_flush \
	packspu_init \
	packspu_misc \
	packspu_net \
	packspu_swapbuf \
	packspu_pixel

ifdef WINDOWS
TRACKS_STATE = 1
LIBRARIES = crutil spuload crmothership 
else
LIBRARIES = crutil spuload crmothership crstate
endif

ifdef BINUTIL_LINK_HACK
TRACKS_STATE = 1
LIBRARIES -= crstate
endif

PACKS = 1

PRECOMP = packspu.c packspu_get.c packspu_flush.c packspu_beginend.c packspu_proto.h
SLOP = $(PRECOMP)

LIB_DEFS += pack.def

include ${TOP}/cr.mk

APIFILES = ../../glapi_parser/apiutil.py ../../glapi_parser/APIspec.txt

packspu.c: pack.py packspu_special packspu_unimplemented_special packspu_proto.h $(APIFILES)
	@$(ECHO) Building the Pack SPU
	@$(PYTHON) pack.py > $@

packspu_get.c: packspu_get.py packspu_special ../../crserverlib/get_sizes.py packspu_proto.h $(APIFILES)
	@$(ECHO) Building the Pack SPU Get functions
	@$(PYTHON) packspu_get.py > $@	

packspu_flush.c: packspu_flush.py packspu_flush_special $(APIFILES)
	@$(ECHO) Building the Pack SPU Auto-flushing functions
	@$(PYTHON) packspu_flush.py > $@

packspu_beginend.c: packspu_beginend.py packspu_vertex_special packspu_proto.h $(APIFILES)
	@$(ECHO) Building the Pack SPU Vertex functions
	@$(PYTHON) packspu_beginend.py > $@

packspu_proto.h: packspu_proto.py packspu_special packspu_unimplemented_special $(APIFILES)
	@$(ECHO) Building the Pack SPU prototypes header
	@$(PYTHON) packspu_proto.py > $@

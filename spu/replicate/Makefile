# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

SPU = 1
SHARED = 1
LIBRARY = replicatespu
FILES = replicatespu \
	replicatespu_beginend \
	replicatespu_bufferobject \
	replicatespu_client \
	replicatespu_config \
	replicatespu_context \
	replicatespu_diffapi \
	replicatespu_generate \
	replicatespu_getstring \
	replicatespu_init \
	replicatespu_lists \
	replicatespu_misc \
	replicatespu_net \
	replicatespu_redirect \
	replicatespu_stateapi \
	replicatespu_swapbuf \
	replicatespu_pixel

ifdef WINDOWS
TRACKS_STATE = 1
LIBRARIES = crutil spuload crmothership 
else
LIBRARIES = crutil spuload crmothership crstate crdlm
endif

LDFLAGS += -lXmu -lX11 -lVncExt

ifdef BINUTIL_LINK_HACK
TRACKS_STATE = 1
LIBRARIES -= crstate
endif

PACKS = 1

PRECOMP = replicatespu.c replicatespu_beginend.c replicatespu_proto.h replicatespu_diffapi.c replicatespu_generate.c replicatespu_stateapi.c
SLOP = $(PRECOMP)

LIB_DEFS += replicate.def

include ${TOP}/cr.mk

APIFILES = ../../glapi_parser/apiutil.py ../../glapi_parser/APIspec.txt

replicatespu.c: replicate.py replicate_special replicatespu_flush_special $(APIFILES) replicatespu_proto.h
	@$(ECHO) Building the Replicate SPU
	@$(PYTHON) replicate.py > $@

replicatespu_diffapi.c: diffapi.py $(APIFILES) replicatespu_diff_special replicatespu_pixel_special
	@$(ECHO) Building the Replicate Differencing API
	@$(PYTHON) diffapi.py > $@

replicatespu_stateapi.c: stateapi.py $(APIFILES)
	@$(ECHO) Building the Replicate State Update API
	@$(PYTHON) stateapi.py > $@

replicatespu_generate.c: replicatespu_generate.py replicate_special replicatespu_flush_special replicatespu_writeback_special $(APIFILES)
	@$(ECHO) Generating Replicate SPU functions
	@$(PYTHON) replicatespu_generate.py > $@

replicatespu_beginend.c: replicatespu_beginend.py replicatespu_vertex_special $(APIFILES) replicatespu_proto.h
	@$(ECHO) Building the Replicate SPU Vertex functions
	@$(PYTHON) replicatespu_beginend.py > $@

replicatespu_proto.h: replicatespu_proto.py replicatespu_special replicatespu_flush_special $(APIFILES)
	@$(ECHO) Building the Replicate SPU prototypes header
	@$(PYTHON) replicatespu_proto.py > $@

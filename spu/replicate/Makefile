# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/arch.mk

SHARED = 1
LIBRARY = replicatespu
FILES = replicatespu \
	replicatespu_beginend \
	replicatespu_bufferobject \
	replicatespu_client \
	replicatespu_config \
	replicatespu_context \
	replicatespu_diffapi \
	replicatespu_get \
	replicatespu_getstring \
	replicatespu_flush \
	replicatespu_init \
	replicatespu_misc \
	replicatespu_net \
	replicatespu_redirect \
	replicatespu_state \
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

PRECOMP = replicatespu.c replicatespu_flush.c replicatespu_state.c replicatespu_beginend.c replicatespu_proto.h replicatespu_diffapi.c
SLOP = $(PRECOMP)

LIB_DEFS += replicate.def

include ${TOP}/cr.mk

APIFILES = ../../glapi_parser/apiutil.py ../../glapi_parser/APIspec.txt

replicatespu.c: replicate.py replicatespu_special replicatespu_unimplemented_special $(APIFILES) replicatespu_state_special replicatespu_get_special replicatespu_proto.h
	@$(ECHO) Building the Replicate SPU
	@$(PYTHON) replicate.py > $@

replicatespu_diffapi.c: diffapi.py $(APIFILES) replicatespu_state_special replicatespu_diff_special
	@$(ECHO) Building the Replicate Differencing API
	@$(PYTHON) diffapi.py > $@

replicatespu_state.c: replicatespu_state.py replicatespu_state_special replicatespu_get_special
	@$(ECHO) Building the Replicate SPU State Tracker
	@$(PYTHON) replicatespu_state.py > $@

replicatespu_flush.c: replicatespu_flush.py replicatespu_flush_special $(APIFILES)
	@$(ECHO) Building the Replicate SPU Auto-flushing functions
	@$(PYTHON) replicatespu_flush.py > $@

replicatespu_beginend.c: replicatespu_beginend.py replicatespu_vertex_special $(APIFILES) replicatespu_proto.h
	@$(ECHO) Building the Replicate SPU Vertex functions
	@$(PYTHON) replicatespu_beginend.py > $@

replicatespu_proto.h: replicatespu_proto.py replicatespu_special replicatespu_unimplemented_special $(APIFILES)
	@$(ECHO) Building the Replicate SPU prototypes header
	@$(PYTHON) replicatespu_proto.py > $@

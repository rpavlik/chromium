# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../

include $(TOP)/arch.mk

LIBRARY = crdlm
LIBRARIES = spuload crutil crmothership
SHARED = 1
FILES = dlm dlm_api dlm_dispatch dlm_pointers dlm_client dlm_arrays
LIB_DEFS = dlm.def

PRECOMP = dlm_dispatch.c dlm_dispatch.h
SLOP = $(PRECOMP)

include $(TOP)/cr.mk

dlm_dispatch.h: dlm_dispatch.py dlm_functions ../glapi_parser/gl_header.parsed
	$(PYTHON) dlm_dispatch.py headers > $@
dlm_dispatch.c: dlm_dispatch.py dlm_functions ../glapi_parser/gl_header.parsed 
	$(PYTHON) dlm_dispatch.py source > $@

dlm_pointers.o: dlm_dispatch.h
dlm_dispatch.c: dlm_dispatch.h

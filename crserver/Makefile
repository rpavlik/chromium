# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

PROGRAM = crserver
FILES = main \
	server_boundsinfo \
	server_clear \
	server_config \
	server_dispatch \
	server_gentextures \
	server_get \
	server_getmap \
	server_getstring \
	server_getpointer \
	server_getpixelmap \
	server_getteximage \
	server_misc \
	server_papi \
	server_projmatrix \
	server_retval \
	server_simpleget \
	server_stream \
	server_viewport \
	server_writeback

PRECOMP = server_dispatch.h server_dispatch.c server_get.c server_retval.c server_simpleget.c
SLOP = ${PRECOMP}

LIBRARIES = spuload crutil crmothership 
TRACKS_STATE = 1
UNPACKS = 1

include $(TOP)/cr.mk

server_dispatch.h: server_dispatch_header.py ../packer/packer_get_special server_special
	@$(ECHO) Building the server dispatch header
	@$(PYTHON) server_dispatch_header.py > $@

server_dispatch.c: server_dispatch.py ../packer/packer_get_special server_special
	@$(ECHO) Building the server dispatch table
	@$(PYTHON) server_dispatch.py > $@

server_retval.c: server_retval.py ../packer/packer_get_special server_special
	@$(ECHO) Building the server retval functions
	@$(PYTHON) server_retval.py > $@

server_get.c: server_get.py ../packer/packer_get_special server_special
	@$(ECHO) Building the server get functions
	@$(PYTHON) server_get.py > $@

server_simpleget.c: server_simpleget.py get_sizes.py
	@$(ECHO) Building the simple server get functions
	@$(PYTHON) server_simpleget.py > $@

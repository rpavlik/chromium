# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

SHARED = 1
LIBRARY = crunpacker
FILES = unpack \
	unpack_arrays \
	unpack_bounds \
	unpack_bufferobject \
	unpack_calllists \
	unpack_clipplane \
	unpack_context \
	unpack_drawpixels \
	unpack_fog \
	unpack_lights \
	unpack_map \
	unpack_materials \
	unpack_matrices \
	unpack_misc \
	unpack_pixelmap \
	unpack_point \
	unpack_program \
	unpack_readpixels \
	unpack_regcombiner \
	unpack_stipple \
	unpack_texture \
	unpack_writeback
LIB_DEFS = unpack.def

LIBRARIES = crutil spuload crpacker

PRECOMP = unpack.c unpack_extend.h
SLOP = $(PRECOMP)

LIB_COPIES = crserver \
	hiddenlinespu \
	injectorspu \
	archsplitspu

include $(TOP)/cr.mk

unpack.c: unpack.py unpacker_special unpack_extend.h ../glapi_parser/apiutil.py ../glapi_parser/APIspec.txt
	@$(ECHO) Making the Unpacker...
	@$(PYTHON) unpack.py > $@

unpack_extend.h: unpacker_special unpack_extend.py ../glapi_parser/apiutil.py ../glapi_parser/APIspec.txt
	@$(PYTHON) unpack_extend.py > $@


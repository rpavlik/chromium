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
	unpack_readpixels \
	unpack_regcombiner \
	unpack_stipple \
	unpack_texture \
	unpack_writeback
LIB_DEFS = unpack.def

LIBRARIES = crutil spuload crpacker

PRECOMP = unpack.c ../include/cr_unpackfuncs.h
SLOP = $(PRECOMP)

LIB_COPIES = crserver \
	hiddenlinespu

include $(TOP)/cr.mk

../include/cr_unpackfuncs.h: unpack_header.py 
	@$(ECHO) Making the Unpack Function header...
	@$(PYTHON) unpack_header.py > $@

unpack.c: unpack.py ../packer/opcode_special ../packer/opcode_extend_special unpacker_special unpacker_vector_special unpacker_extend_special
	@$(ECHO) Making the UnPacker...
	@$(PYTHON) unpack.py > $@

TOP = ..

SHARED = 1
LIBRARY = crpacker
FILES = packer \
	pack_arrays \
	pack_bbox \
	pack_bounds \
	pack_buffer \
	pack_clipplane \
	pack_context \
	pack_current \
	pack_fog \
	pack_lights \
	pack_lists \
	pack_map \
	pack_materials \
	pack_matrices \
	pack_pixelmap \
	pack_pixels \
	pack_rect \
	pack_stipple \
	pack_texture
LIB_DEFS = packer.def

LIBRARIES = crutil

PRECOMP = packer.def ../include/cr_packfunctions.h ../include/cr_opcodes.h ../include/state/cr_currentpointers.h packer.c
SLOP = $(PRECOMP)

include $(TOP)/cr.mk

packer.def: packer_defs.py
	@$(ECHO) Making the packer def file
	@$(PYTHON) $< > $@

../include/cr_packfunctions.h: pack_header.py packer_get_special packer_pixel_special
	@$(ECHO) Making the pack functions header..
	@$(PYTHON) pack_header.py > $@

../include/cr_opcodes.h: opcodes.py opcode_special packer_get_special
	@$(ECHO) Making the opcode header...
	@$(PYTHON) opcodes.py > $@

../include/state/cr_currentpointers.h: pack_currentheader.py pack_currenttypes.py
	@$(ECHO) Making cr_currentpointers.h
	@$(PYTHON) pack_currentheader.py > $@

packer.c: packer.py packer_special packer_get_special
	@$(ECHO) Making the Packer...
	@$(PYTHON) packer.py > $@

pack_bbox.c: packer_bbox.py
	@$(ECHO) Making the BBOX Packer...
	@$(PYTHON) packer_bbox.py > $@

pack_current.c: pack_current.py pack_currenttypes.py
	@$(ECHO) Making the current functions...
	@$(PYTHON) pack_current.py > $@

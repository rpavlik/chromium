TOP = ..

SHARED = 1
LIBRARY = crpacker
FILES = packer pack_arrays pack_buffer pack_clipplane pack_fog \
	pack_lights pack_lists pack_map pack_materials pack_matrices \
	pack_pixelmap pack_rect pack_stipple pack_texture
LIB_DEFS = packer.def

LIBRARIES = crutil

PRECOMP = packer.def ../include/cr_opcodes.h packer.c
SLOP = $(PRECOMP)

include $(TOP)/cr.mk

packer.def: packer_defs.py
	@$(ECHO) Making the packer def file
	@$(PYTHON) $< > $@

../include/cr_opcodes.h: opcodes.py opcode_special packer_get_special
	@$(ECHO) Making the opcode header...
	@$(PYTHON) opcodes.py > $@

packer.c: packer.py packer_special packer_get_special
	@$(ECHO) Making the Packer...
	@$(PYTHON) packer.py > $@

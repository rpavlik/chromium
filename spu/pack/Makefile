TOP = ../..

SHARED = 1
LIBRARY = packspu
FILES = packspu \
	packspu_client \
	packspu_config \
	packspu_get \
	packspu_getstring \
	packspu_init \
	packspu_net \
	packspu_pixel \
	packspu_swap

LIBRARIES = crutil crmothership crstate crpacker

PRECOMP = packspu.c packspu_get.c packspu_pixel.c 
SLOP = $(PRECOMP)

LIB_DEFS += pack.def
include ${TOP}/cr.mk

packspu.c: pack.py packspu_pixel_special packspu_special packspu_unimplemented_special
	@$(ECHO) Building the Pack SPU
	@$(PYTHON) pack.py > $@
	
packspu_get.c: packspu_get.py
	@$(ECHO) Building the Pack SPU Get functions
	@$(PYTHON) packspu_get.py > $@	

packspu_pixel.c: packspu_pixel.py packspu_pixel_special
	@$(ECHO) Building the Pack SPU Pixel functions
	@$(PYTHON) packspu_pixel.py > $@

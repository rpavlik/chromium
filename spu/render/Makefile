TOP = ../..

LIBRARY = renderspu
FILES = renderspu renderspu_config renderspu_init renderspu_swap renderspu_window

LIBRARIES = crmothership crutil

PRECOMP = renderspu.c
SLOP = $(PRECOMP)

LIB_DEFS += render.def
include ${TOP}/cr.mk

ifdef WINDOWS
LDFLAGS += user32.lib gdi32.lib
endif

renderspu.c: render.py render_special
	@$(ECHO) Building the Render SPU
	@$(PYTHON) render.py > renderspu.c

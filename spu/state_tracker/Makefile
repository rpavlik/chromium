TOP = ../..
SHARED = 1
LIBRARY = crstate
LIBRARIES = spuload crutil

FILES = state_buffer \
	state_client \
	state_current \
	state_diff \
	state_enable \
	state_error \
	state_evaluators \
	state_flush \
	state_fog \
	state_get \
	state_getstring \
	state_init \
	state_isenabled \
	state_lighting \
	state_line \
	state_lists \
	state_pixel \
	state_polygon \
	state_rasterpos \
	state_stencil \
	state_texture \
	state_transform \
	state_viewport
LIB_DEFS = state.def

PRECOMP = state_get.c state_isenabled.c state.def ../../include/state/cr_statefuncs.h
SLOP = $(PRECOMP)

include $(TOP)/cr.mk

state_get.c: state_get.py state_get.txt
	@$(ECHO) Making the Get functions
	@$(PYTHON) state_get.py > $@

state_isenabled.c: state_isenabled.py state_isenabled.txt
	@$(ECHO) Making the IsEnabled function
	@$(PYTHON) state_isenabled.py > $@

state.def: state_defs.py state_special
	@$(ECHO) Making state.def
	@$(PYTHON) state_defs.py > $@

../../include/state/cr_statefuncs.h: state_funcs.py state_special
	@$(ECHO) Making cr_statefuncs.h
	@$(PYTHON) state_funcs.py > $@

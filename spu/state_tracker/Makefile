TOP = ../..
SHARED = 1
LIBRARY = crstate
LIBRARIES = spuload crutil

REGULAR_FILES = state_buffer \
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

DIFF_FILES = buffer \
	client \
	fog \
	lighting \
	line \
	pixel \
	polygon \
	stencil \
	viewport

DIFF_FILES1 = $(addprefix state_, $(DIFF_FILES))
DIFF_FILES2 = $(addsuffix _gen, $(DIFF_FILES1))

# They're in this order because I'm debugging the differencing files
# and it's irritating to have to wait for the whole compile to get
# to them.  -Greg
FILES = $(DIFF_FILES2) $(REGULAR_FILES)

LIB_DEFS = state.def

PRECOMP = state_get.c state_isenabled.c state.def ../../include/state/cr_statefuncs.h
PRECOMP += $(addsuffix .c, $(DIFF_FILES2)) 
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

state_buffer_gen.c: state_buffer.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Buffer > $@

state_client_gen.c: state_client.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Client > $@

state_fog_gen.c: state_fog.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Fog > $@

state_lighting_gen.c: state_lighting.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Lighting > $@

state_line_gen.c: state_line.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Line > $@

state_pixel_gen.c: state_pixel.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Pixel > $@

state_polygon_gen.c: state_polygon.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Polygon > $@

state_stencil_gen.c: state_stencil.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Stencil > $@

state_viewport_gen.c: state_viewport.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Viewport > $@


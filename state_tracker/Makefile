# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../
LIBRARY = crstate
LIBRARIES = spuload crutil crmothership

REGULAR_FILES = state_attrib \
	state_buffer \
	state_client \
	state_current \
	state_diff \
	state_enable \
	state_error \
	state_evaluators \
	state_extensions \
	state_flush \
	state_fog \
	state_get \
	state_getstring \
	state_hint \
	state_init \
	state_isenabled \
	state_lighting \
	state_limits \
	state_line \
	state_lists \
	state_pixel \
	state_point \
	state_polygon \
	state_rasterpos \
	state_regcombiner \
	state_stencil \
	state_texture \
	state_transform \
	state_viewport

DIFF_FILES = buffer \
	current \
	fog \
	hint \
	lighting \
	line \
	pixel \
	point \
	polygon \
	regcombiner \
	stencil \
	viewport

DIFF_FILES1 = $(addprefix state_, $(DIFF_FILES))
DIFF_FILES2 = $(addsuffix _gen, $(DIFF_FILES1))

FILES = $(REGULAR_FILES) $(DIFF_FILES2)

LIB_DEFS = state.def

PRECOMP = state_get.c state_isenabled.c state.def ../include/state/cr_statefuncs.h
PRECOMP += $(addsuffix .c, $(DIFF_FILES2)) 
SLOP = $(PRECOMP)

ifdef WINDOWS
SHARED = 1
LIB_COPIES = crserver \
	packspu \
	tilesortspu \
	arrayspu \
	hiddenlinespu \
	nopspu \
	simplequeryspu \
	binaryswapspu \
	readbackspu \
	statecopytest
else
# make static libcrstate.a library on everything else
endif


include $(TOP)/cr.mk

state_get.c: state_get.py state_get.txt state_extensions_get.txt
	@$(ECHO) Making the Get functions
	@$(PYTHON) state_get.py > $@

state_isenabled.c: state_isenabled.py state_isenabled.txt state_extensions_isenabled.txt
	@$(ECHO) Making the IsEnabled function
	@$(PYTHON) state_isenabled.py > $@

state.def: state_defs.py state_special
	@$(ECHO) Making state.def
	@$(PYTHON) state_defs.py > $@

../include/state/cr_statefuncs.h: state_funcs.py state_special
	@$(ECHO) Making cr_statefuncs.h
	@$(PYTHON) state_funcs.py > $@

state_buffer_gen.c: state_buffer.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Buffer > $@

state_client_gen.c: state_client.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Client > $@

state_current_gen.c: state_current.py convert.py
	@$(ECHO) Making $@
	@$(PYTHON) state_current.py > $@

state_fog_gen.c: state_fog.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Fog > $@

state_hint_gen.c: state_hint.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Hint > $@

state_lighting_gen.c: state_lighting.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Lighting > $@

state_line_gen.c: state_line.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Line > $@

state_pixel_gen.c: state_pixel.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Pixel > $@

state_point_gen.c: state_point.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Point > $@

state_polygon_gen.c: state_polygon.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Polygon > $@

state_regcombiner_gen.c: state_regcombiner.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl RegCombiner > $@

state_stencil_gen.c: state_stencil.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Stencil > $@

state_viewport_gen.c: state_viewport.txt gendiffcode.pl
	@$(ECHO) Making $@
	@$(PERL) gendiffcode.pl Viewport > $@


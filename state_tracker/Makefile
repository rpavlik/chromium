# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../

include $(TOP)/arch.mk

LIBRARY = crstate
LIBRARIES = spuload crutil crmothership

REGULAR_FILES = state_attrib \
	state_buffer \
	state_bufferobject \
	state_client \
	state_current \
	state_diff \
	state_enable \
	state_error \
	state_evaluators \
	state_feedback \
	state_fence \
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
	state_multisample \
	state_occlude \
	state_pixel \
	state_point \
	state_polygon \
	state_program \
	state_rasterpos \
	state_regcombiner \
	state_stencil \
	state_texture \
	state_teximage \
	state_transform \
	state_viewport

DIFF_FILES = buffer \
	current \
	fog \
	hint \
	lighting \
	line \
	multisample \
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
LIB_COPIES = crserverlib \
	packspu \
	tilesortspu \
	arrayspu \
	hiddenlinespu \
	feedbackspu \
	nopspu \
	simplequeryspu \
	pixelsortspu \
	statecopytest
else
# make static libcrstate.a library on everything else
endif

ifdef BINUTIL_LINK_HACK
SHARED = 1
LIB_COPIES = crserverlib \
	packspu \
	tilesortspu \
	arrayspu \
	hiddenlinespu \
	feedbackspu \
	nopspu \
	simplequeryspu \
	renderspu \
	statecopytest
endif


include $(TOP)/cr.mk

APIFILES = ../glapi_parser/apiutil.py ../glapi_parser/APIspec.txt

state_get.c: state_get.py state_get.txt state_extensions_get.txt $(APIFILES)
	@$(ECHO) Making the Get functions
	@$(PYTHON) state_get.py > $@

state_isenabled.c: state_isenabled.py state_isenabled.txt state_extensions_isenabled.txt $(APIFILES)
	@$(ECHO) Making the IsEnabled function
	@$(PYTHON) state_isenabled.py > $@

state.def: state_defs.py state_special state_select_special state_feedback_special $(APIFILES)
	@$(ECHO) Making state.def
	@$(PYTHON) state_defs.py > $@

../include/state/cr_statefuncs.h: state_funcs.py state_special state_feedback_special state_select_special $(APIFILES)
	@$(ECHO) Making cr_statefuncs.h
	@$(PYTHON) state_funcs.py > $@

state_buffer_gen.c: state_buffer.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py buffer Buffer > $@

state_client_gen.c: state_client.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py client Client > $@

state_current_gen.c: state_current.py convert.py $(APIFILES)
	@$(ECHO) Making $@
	@$(PYTHON) state_current.py > $@

state_fog_gen.c: state_fog.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py fog Fog > $@

state_hint_gen.c: state_hint.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py hint Hint > $@

state_lighting_gen.c: state_lighting.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py lighting Lighting > $@

state_line_gen.c: state_line.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py line Line > $@

state_multisample_gen.c: state_multisample.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py multisample Multisample >$@

state_point_gen.c: state_point.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py point Point > $@

state_polygon_gen.c: state_polygon.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py polygon Polygon > $@

state_regcombiner_gen.c: state_regcombiner.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py regcombiner RegCombiner > $@

state_stencil_gen.c: state_stencil.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py stencil Stencil > $@

state_viewport_gen.c: state_viewport.txt gendiffcode.py
	@$(ECHO) Making $@
	@$(PYTHON) gendiffcode.py viewport Viewport > $@


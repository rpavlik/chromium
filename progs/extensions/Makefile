# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

ifneq ($(ARCH), WIN_NT)
ifneq ($(ARCH), WIN_98)
ARCH=$(shell uname | sed -e 's/-//g')
endif
endif

SUBDIRS = common \
	ARB_multitexture \
	ARB_multitexture2 \
	ARB_texture_cube_map \
	EXT_blend_color \
	EXT_blend_minmax \
	EXT_blend_subtract \
	EXT_separate_specular_color \
	EXT_texture_edge_clamp \
	EXT_texture_filter_anisotropic \
	NV_fog_distance

ifneq ($(ARCH), AIX)
SUBDIRS+=       EXT_secondary_color
endif

include ${TOP}/cr.mk

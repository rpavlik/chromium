# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..
PRECOMP = gl_header.parsed
SLOP = $(PRECOMP)

BANNER = Just parsing the OpenGL header file!

include ${TOP}/cr.mk

gl_header.parsed: parser.py system_header/GL.H system_header/CR_GL.H
	@$(ECHO) "Here we go..."
	@$(PYTHON) parser.py

TOP = ..
PRECOMP = gl_header.parsed
SLOP = $(PRECOMP)

BANNER = Just parsing the OpenGL header file!

include ${TOP}/cr.mk

gl_header.parsed: parser.py system_header/GL.H system_header/CR_GL.H
	@$(ECHO) "Here we go..."
	@$(PYTHON) parser.py

TOP = ..
PRECOMP = gl_header.parsed
SLOP = $(PRECOMP)

BANNER = Just parsing the OpenGL header file!

include ${TOP}/cr.mk

gl_header.parsed: parser.py
	@$(ECHO) "Here we go..."
	@$(PYTHON) parser.py

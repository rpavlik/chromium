# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

# The loader has to come first because it builds 
# the dispatch table header.  I know that sucks.

SUBDIRS = tester
SUBDIRS += error nop pack passthrough print readback render template tilesort
SUBDIRS += saveframe comm array binaryswap counter hiddenline dist_texture

include ${TOP}/cr.mk

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

# The loader has to come first because it builds 
# the dispatch table header.  I know that sucks.

SUBDIRS = loader tester state_tracker
SUBDIRS += error nop pack passthrough print readback render template tilesort
SUBDIRS += saveframe comm array binaryswap

include ${TOP}/cr.mk

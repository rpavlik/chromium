# Copyright (c) 2004-2005, Tungsten Graphics, Inc.
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include ${TOP}/options.mk

SPU = 1
SHARED = 1
LIBRARY = vncspu
FILES = vncspu \
	vncspu_config \
	vncspu_init \
	actions \
	active \
	async_io \
	client_io \
	control \
	d3des \
	decode_hextile \
	decode_tight \
	encode \
	encode_tight \
	fbs_files \
	host_connect \
	host_io \
	logging \
	main \
	region \
	region_more \
	rfblib \
	translate

#LDFLAGS += -lXmu -lX11 -lVncExt
LDFLAGS += -lX11 -lVncExt

CFLAGS += -DCHROMIUM -DRASTER_BOTTOM_TO_TOP=1
LDFLAGS += -lz -ljpeg -L/usr/X11R6/lib -lX11

CFLAGS += -DHAVE_XCLIPLIST_EXT
LDFLAGS += -lXcliplist

ifeq ($(USE_VNC), 1)
CFLAGS += -DHAVE_VNC_EXT
LDFLAGS += -lVncExt
endif

LIBRARIES = spuload crutil crmothership

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += vncspu.def
include ${TOP}/cr.mk

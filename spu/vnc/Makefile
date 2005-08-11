# Copyright (c) 2004-2005, Tungsten Graphics, Inc.
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

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

LIBRARIES = spuload crutil crmothership z jpeg

PRECOMP =
SLOP = $(PRECOMP)

LIB_DEFS += vncspu.def
include ${TOP}/cr.mk

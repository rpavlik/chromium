# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..

include $(TOP)/cr.mk

all:
	@echo Generating crconfig.py
	@echo "arch = \"${ARCH}\"" > crconfig.py
	@echo "crdir = \"$(shell dirname $(shell dirname $(shell pwd)))\"" >> crconfig.py

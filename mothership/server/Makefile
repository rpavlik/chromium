# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..
PRECOMP = crconfig.py
SLOP = $(PRECOMP)

BANNER = Making your local configuration file

include $(TOP)/cr.mk

crconfig.py:
	@echo Generating crconfig.py
	@echo "arch = \"${ARCH}\"" > crconfig.py
	@echo "crdir = \"$(shell dirname $(shell dirname $(shell pwd)))\"" >> crconfig.py

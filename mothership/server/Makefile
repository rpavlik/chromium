# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ../..
PRECOMP = crconfig.py
SLOP = $(PRECOMP)

BANNER = Making your local configuration file

include $(TOP)/cr.mk

crconfig.py: Makefile
	@echo Generating crconfig.py
	@echo "import os" > crconfig.py
	@echo "arch = \"${ARCH}\"" >> crconfig.py
ifdef WINDOWS
	@echo "crdir = r\"$(shell cygpath -w $(shell dirname $(shell dirname $(shell pwd))))\"" >> crconfig.py
	@echo "crbindir = os.path.join(crdir,'bin',arch)" >> crconfig.py
	@echo "crlibdir = os.path.join(crdir,'bin',arch)" >> crconfig.py
else
	@echo "crdir = \"$(shell dirname $(shell dirname $(shell pwd)))\"" >> crconfig.py
	@echo "crbindir = os.path.join(crdir,'bin',arch)" >> crconfig.py
	@echo "crlibdir = os.path.join(crdir,'lib',arch)" >> crconfig.py
endif

TOP = ..

PROGRAM = crserver
FILES = main server_config server_dispatch server_get server_retval

PRECOMP = server_dispatch.h server_dispatch.c server_retval.c
SLOP = ${PRECOMP}

LIBRARIES = spuload crunpacker crutil crmothership

include $(TOP)/cr.mk

server_dispatch.h: server_dispatch_header.py ../packer/packer_get_special server_special
	@$(ECHO) Building the server dispatch header
	@$(PYTHON) server_dispatch_header.py > $@

server_dispatch.c: server_dispatch.py ../packer/packer_get_special server_special
	@$(ECHO) Building the server dispatch table
	@$(PYTHON) server_dispatch.py > $@

server_retval.c: server_retval.py ../packer/packer_get_special server_special
	@$(ECHO) Building the server retval functions
	@$(PYTHON) server_retval.py > $@

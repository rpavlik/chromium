TOP = .
SUBDIRS = glapi_parser util state_tracker packer unpacker mothership spu \
	app_faker opengl_stub crserver

include ${TOP}/cr.mk

TOP = .
SUBDIRS = glapi_parser util state_tracker packer spu unpacker \
	app_faker opengl_stub crserver mothership

include ${TOP}/cr.mk

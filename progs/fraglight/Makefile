TOP = ../..

include $(TOP)/arch.mk

PROGRAM = fraglight
FILES = fraglight
GLUT = 1
MATH = 1
LIBRARIES = crutil crfaker crmothership spuload

ifdef DARWIN
LDFLAGS  += -multiply_defined suppress -bind_at_load
endif

include $(TOP)/cr.mk

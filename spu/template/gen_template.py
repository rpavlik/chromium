# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import os, os.path
import sys

if os.getcwd().find( "template" ) != -1:
	print >> sys.stderr, "You're running this script in the template directory, which I'm SURE you don't want to do!"
	sys.exit(-1)

if len( sys.argv ) != 2:
	print >> sys.stderr, "Usage: %s <SPUNAME>" % sys.argv[0]
	sys.exit(-1)

spuname = sys.argv[1]

def ProcessFile( fn, spuname ):
	print >> sys.stderr, "Processing file %s..." % fn
	inputfile = open( fn, 'r' )
	outputfile = open( fn + "_TEMP", 'w' )
	for line in inputfile.readlines():
		newline = line.replace( "template", spuname )
		newline = newline.replace( "TEMPLATE", spuname.upper() )
		newline = newline.replace( "Template", (spuname[:1].upper() + spuname[1:]) )
		outputfile.write( newline )
	inputfile.close()
	outputfile.close()
	os.unlink( fn )
	os.rename( fn + "_TEMP", fn.replace( "template", spuname ) )

ProcessFile( "templatespu.c", spuname )
ProcessFile( "templatespu.h", spuname )
ProcessFile( "templatespu_config.c", spuname )
ProcessFile( "templatespu_init.c", spuname )
ProcessFile( "templatespu.def", spuname )
ProcessFile( "templatespu_proto.py", spuname )
ProcessFile( "Makefile", spuname )

# File: fastdep.py
# Author: Christopher R. Waters <crw7@msstate.edu>
# Last Modified: 2004/09/30 3:00pm CST

""" 'Fast' dependency generation

Initial version: direct port from perl (fastdep.pl)
Current version: geared more towards actual speed.
"""

import sys, getopt, os
from os.path import dirname, exists, splitext

dInc = dict()

def includes( incfile ):
	global dInc

	if incfile in dInc:
		return dInc[ incfile ]

	try: hFile = file( incfile )
	except ( IOError, TypeError ):
		return None

	angles = []
	quotes = []

	for line in hFile:
		line = line.lstrip()
		if len(line) == 0 or line[0] != '#':
			continue

		line = line[1:].lstrip()
		if not line.startswith( "include" ):
			continue

		line = line[7:].lstrip().rstrip()
		if len(line) == 0: continue

		if line[0] == '<' and line[-1] == '>':
			angles.append( line[1:-1] )
		elif line[0] == '\"' and line[-1] == '\"':
			quotes.append( line[1:-1] )
		# else invalid

	hFile.close()

	incdir = dirname( incfile ) + '/'
	files = []

	for name in quotes:
		f = incdir + name
		if exists( f ):
			files.append( f )
		else:
			angles.append( name )

	for name in angles:
		for incdir in incpath:
			f = incdir + name
			if exists( f ):
				files.append( f )
				break

	dInc[ incfile ] = files

	return files

def depends( depfile ):
	depfiles = [ depfile ]
	dFiles = dict()

	while len( depfiles ):
		_file = depfiles.pop()
		if _file in dFiles:
			continue

		dFiles[ _file ] = 1
		depfiles += includes( _file )

	return dFiles.keys()


obj_prefix = ""

if os.name == 'nt': obj_suffix = ".obj"
else:               obj_suffix = ".o"

extra_targets = []
incpath = []
files = []

"""Options accepted:
	-I<include path>
	--obj-prefix=<prefix>
	--obj-suffix=<suffix>
	--extra-target=<targets>
	<files>
"""
optlist, args = getopt.getopt( sys.argv[1:], 'I:', ['obj-prefix=', 'obj-suffix=', 'extra-target='] )
files = [ arg for arg in args if arg[0] != '/' ]

for option, val in optlist:
	if len(val) == 0: continue

	if option == '-I':
		if val[-1] != '/': val += '/'
		incpath.append( val )

	elif option == '--obj-prefix':
		obj_prefix = val

	elif option == '--obj-suffix':
		obj_suffix = val

	elif option == '--extra-target':
		extra_targets.append( val )

# Remove any quotes.
if obj_prefix[0] in ('\'','\"'): obj_prefix = obj_prefix.replace( obj_prefix[0], '' )
if obj_suffix[0] in ('\'','\"'): obj_suffix = obj_suffix.replace( obj_suffix[0], '' )

for incfile in files:
	val = splitext( incfile )[0] # strip the extension
	obj = obj_prefix + val + obj_suffix

	for t in extra_targets:
		obj += " " + t

	for incfile in depends( incfile ):
		print "%s: %s" % ( obj, incfile )

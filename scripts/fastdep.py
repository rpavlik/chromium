
import sys
import re
import os

def directory( dfile ):
	if "/" in dfile:
		d = re.match("^(.*)\/([^\/])+$", dfile ).group(1)
	else:
		d = "."

	return ( d + "/" )

dInc = dict()

def includes( incfile ):
	global dInc

	if incfile in dInc:
		return dInc[ incfile ]

	try: hFile = file( incfile )
	except (IOError, TypeError):
		return None

	angles = []
	quotes = []

	for line in hFile.readlines():
		if re.match("\s*\#", line) == None:
			continue
		m = re.match("^\s*#\s*include\s*([<\"])(.*)[>\"]", line)
		if m:
			if m.group(1) == "<":
				angles.append( m.group(2) )
			else:
				quotes.append( m.group(2) )

	hFile.close()

	incdir = directory(incfile)
	files = []

	while len(quotes):
		name = quotes.pop()
		f = incdir + name
		if os.path.exists(f):
			files.append( f )
		else:
			angles.append( name )

	for name in angles:
		for incdir in incpath:
			f = incdir + name
			if os.path.exists(f):
				files.append(f)
				break

	dInc[incfile] = files

	return files

def depends( depfile ):
	depfiles = [depfile]
	dFiles = dict()

	while len(depfiles):
		f = depfiles.pop()
		if f in dFiles:
			continue

		dFiles[f] = 1
		f = includes(f)
		if f: depfiles[:0] = f

	return dFiles.keys()


obj_prefix = ""

if os.name == 'nt': obj_suffix = ".obj"
else:               obj_suffix = ".o"

extra_targets = []
incpath = []
files = []

for arg in sys.argv[1:]:
	val = re.match(r"^-I(.+/)$", arg)
	if val:
		incpath.append( val.group(1) )
		continue

	val = re.match("^-I(.+)$", arg)
	if val:
		incpath.append( val.group(1) + "/" )
		continue

	val = re.match("^--obj-prefix=(.*)$", arg )
	if val:
		obj_prefix = val.group(1)
		continue

	val = re.match("^--obj-suffix=(.*)$", arg )
	if val:
		obj_suffix = val.group(1)
		continue

	val = re.match("^--extra-target=(.*)$", arg )
	if val:
		extra_targets.append(val.group(1))
		continue

	if arg[0] != '/':
		files += [arg]

for incfile in files:
	val = re.match("^(.*)\.\w+$", incfile )
	obj = obj_prefix + val.group(1) + obj_suffix

	for t in extra_targets:
		obj += " " + t

	for incfile in depends(incfile):
		print "%s: %s" % (obj, incfile)

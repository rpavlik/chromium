# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

"""Functions for reading and writing Chromium config files."""

from wxPython.wx import *
import os, re, string, sys, traceback
sys.path.append("../server")
import crtypes, crutils
from crconfig import *


#----------------------------------------------------------------------
# Config file writing


__ConfigFileImports = """# Chromium configuration produced by configtool.py
import string
import sys
sys.path.append( "../server" )
from mothership import *

"""

__ConfigFileHeader = """

# Look for autostart option
for (name, value) in MOTHERSHIP_OPTIONS:
	if name == "auto_start":
		AUTO_START = value

# Get program name
if len(sys.argv) == 1:
	program = DEFAULT_APP
elif len(sys.argv) == 2:
	program = sys.argv[1]
else:
	print "Usage: %s <program>" % sys.argv[0] 
	sys.exit(-1)
if program == "":
	print "No program to run!"
	sys.exit(-1)


cr = CR()

"""

__ConfigFileTail = """
# Set mothership params
for (name, value) in MOTHERSHIP_OPTIONS:
	cr.Conf(name, value)
cr.Go()
"""

def contains(list, item):
	"""Return 1 if item is in list, 0 otherwise."""
	try:
		i = list.index(item)
	except:
		return 0
	else:
		return 1


def WriteConfs(optionList, prefix, file):
	"""Write a prefix.Conf('param', value) line to the file handle for each of
	the options in the given OptionList.  Only write non-default options."""
	for opt in optionList:
		if opt.Value != opt.Default:
			opt.Write(file, "%s.Conf" % prefix, "")
	
def WriteAppConfs(appNode, prefix, file):
	"""Write the .Conf() lines for an app node.  We need to do a few things
	special here (like crbindir)."""
	assert isinstance(appNode, crtypes.ApplicationNode)
	for opt in appNode.GetOptions():
		if opt.Name == "start_dir":
			if opt.Value[0] == crbindir:
				file.write('%s("start_dir", crbindir)\n' % prefix)
			else:
				file.write('%s("start_dir", "%s")\n' % (prefix, opt.Value[0]))
		elif opt.Name == "application":
			file.write('%s("application", DEFAULT_APP)\n' % prefix)
		elif opt.Value != opt.Default:
			opt.Write(file, prefix, "")



def WriteAutoStart(nodeName, isServer, hostName, file):
	"""Write the tricky autostart code for the given node on given host"""
	# XXX this needs more work:
	# 1. support ssh for remote hosts
	# 2. support Windows, how?
	
	# build the AutoStart() argument list
	argList = '[ '
	if hostName == "localhost" or hostName == "":
		# spawn on local host
		argList += '"/bin/sh", "-c", '
		argList += '"LD_LIBRARY_PATH=%s MOTHERSHIP=localhost '
		if isServer:
			argList += '%s/crserver" % (crlibdir, crbindir)'
		else:
			argList += '%s/crappfaker" % (crlibdir, crbindir)'
		argList += ' ]'
	else:
		# span on remote host
		argList += '"/usr/bin/rsh", '
		argList += '"' + hostName + '"'
		if isServer:
			argList += """ "/bin/sh -c 'DISPLAY=:0.0  CRMOTHERSHIP=localhost  LD_LIBRARY_PATH=%s  %s/crserver'" % (crlibdir, crbindir)"""
		else:
			argList += """ "/bin/sh -c 'DISPLAY=:0.0  CRMOTHERSHIP=localhost  LD_LIBRARY_PATH=%s  %s/crappfaker'" % (crlibdir, crbindir)"""
		argList += ' ]'

	# Now, write the code to the file
	file.write("if AUTO_START:\n")
	file.write("\t%s.AutoStart( %s )\n" % (nodeName, argList))


def WriteConfig(mothership, file):
	"""Write the mothership config to file handle."""

	#boilerplate
	file.write(__ConfigFileImports)

	# write mothership/global options
	mothership.GetOptions().Write(file, "MOTHERSHIP_OPTIONS")

	# look for default application to run
	application = ""
	for node in mothership.Nodes():
		if node.IsAppNode():
			app = node.GetOption('application')
			assert len(app) > 0  # app is a list!
			if app[0] != "":
				application = app[0]
				break
	file.write('DEFAULT_APP = "%s"' % application)

	# boilerplate
	file.write(__ConfigFileHeader)

	# Assign an index to each node (needed for AddServer)
	numNodes = 0
	for node in mothership.Nodes():
		node.index = numNodes
		numNodes += node.GetCount()

	# "declare" the nodes array
	file.write("nodes = range(%d)\n\n" % numNodes)

	# write the code to allocate the nodes
	# XXX implement grouping/compound nodes!!!!!!
	i = 0
	for node in mothership.Nodes():
		# emit N nodes
		for j in range(node.GetCount()):
			if node.IsServer():
				file.write("nodes[%d] = CRNetworkNode('%s')\n" %
						   (i, node.GetHosts()[j]))
				WriteConfs(node.GetOptions(), "nodes[%d]" % i, file)
			else:
				# application node
				file.write("nodes[%d] = CRApplicationNode('%s')\n" %
						   (i, node.GetHosts()[j]))
				WriteAppConfs(node, "nodes[%d].Conf" % i, file)


			(x, y) = node.GetPosition()
			file.write("nodes[%d].SetPosition(%d, %d)\n" % (i, x, y))
			file.write("cr.AddNode(nodes[%d])\n" % i)

			WriteAutoStart("nodes[%d]" % i,
						   node.IsServer(),
						   node.GetHosts()[j],
						   file)

			file.write("\n")
			i += 1
		#endfor
	#endfor
	file.write("\n")

	# write the SPUs for each node
	freeport = 7000
	for node in mothership.Nodes():
		for j in range(node.GetCount()):
			numSPUs = len(node.SPUChain())
			if numSPUs > 0:
				if node.IsServer():
					file.write("# network nodes[%d]\n" % (node.index + j))
				else:
					file.write("# application nodes[%d]\n" % (node.index + j))
				file.write("spus = range(%d)\n" % numSPUs)
				k = 0
				for spu in node.SPUChain():
					file.write("spus[%d] = SPU('%s')\n" % (k, spu.Name()))
					WriteConfs(spu.GetOptions(), "spus[%d]" % k, file)
					if k + 1 == numSPUs:
						# last SPU, add servers, if any
						for server in node.GetServers():
							file.write(("spus[%d].AddServer(nodes[%d], " +
									   "protocol='tcpip', port=%d)\n") %
									   (k, server.index, freeport))
							freeport += 1
					file.write("nodes[%d].AddSPU(spus[%d])\n" %
							   (node.index + j, k))
					k += 1
			#endif
			if node.IsServer():
				# write the tile information
				tiles = node.GetTiles(j)
				for tile in tiles:
					file.write("nodes[%d].AddTile(%d, %d, %d, %d)\n" %
					   (node.index + j, tile[0], tile[1], tile[2], tile[3]))
			#endif
			file.write("\n")
		#endif
	#endfor

	file.write("\n")

	# tail of file, boilerplate
	file.write(__ConfigFileTail)
	return 1


#----------------------------------------------------------------------
# Config file writing

TargetMothership = 0

# The following functions emulate class instantiation while reading
# Chromium conf files.  Eventually we'll probably merge the classes
# in crtypes.py with the mothership.py classes.

def CR():
	"""Create a mothership."""
	global TargetMothership
	assert TargetMothership != 0
	return TargetMothership

def CRApplicationNode(host="localhost"):
	"""Create an application node."""
	n = crtypes.ApplicationNode(hostnames=[host], count=1)
	return n

def CRNetworkNode(host="localhost"):
	"""Create a network/server node."""
	n = crtypes.NetworkNode(hostnames=[host], count=1)
	return n

def SPU(name):
	s = crutils.NewSPU(name)
	return s

def GetArgv():
	# prompt the user for an argument list???
	return ["config.py", "atlantis"]

def ReadConfig(mothership, file, filename=""):
	"""Read a mothership config from file handle."""
	global TargetMothership
	TargetMothership = mothership

	# read entire file into a string
	file.seek(0, 0)
	contents = file.read(-1)

	# Remove some lines which cause trouble
	skipPatterns = [ "^sys\.path\.append.*$",
					 "^from mothership import.*$" ]
	for pat in skipPatterns:
		v = re.search(pat, contents, flags=re.MULTILINE)
		if v:
			#print "Ignoring line: %s" % contents[v.start() : v.end()]
			# comment-out the offending line (preserves line numbering)
			contents = contents[0 : v.start()] + "#" + contents[v.start() : ]

	# save copy of original sys.argv list
	origArgv = sys.argv[:]

	(directory, name) = os.path.split(filename)

	# check if sys.argv is referenced
	if re.search("sys\.argv", contents):
		dialog = wxTextEntryDialog(parent=NULL, message=
					"The configuration you're about to load uses the " +
					"sys.argv argument vector.\n" +
					"You may have to provide suitable arguments now.\n" +
					"Example: %s atlantis tilesort" % name,
					caption="Configuration File Arugments",
					defaultValue=name + " ")
		if dialog.ShowModal() == wxID_CANCEL:
			return 0

		# Set new argument vector
		newArgv = dialog.GetValue()
		newArgv = string.split(newArgv)
		sys.argv = newArgv
		dialog.Destroy()

	exec contents
		
	# Try to execute the config file
	try:
		pass
#		exec contents
	except:
		# get exception info
		(type, value, callStack) = sys.exc_info()
		# extract traceback information
		callList = traceback.extract_tb(callStack)
		# we only care about the top-most record on call stack
		callList = [ callList[-1] ]
		assert len(callList) == 1
		# convert to a string representation
		strList = traceback.format_list(callList)
		assert len(strList) == 1
		# find line number in strList[0]
		v = re.search("line (\d+)", strList[0])
		if v:
			line = int(strList[0][v.start(1) : v.end(1)])
		else:
			line = 0
		dialog = wxMessageDialog(parent=NULL,
			message = "There was a problem while reading the config file.\n" +
			('"%s" on line %d of %s' % (str(type), line, name)),
			caption="File Parsing Problem",
			style=wxOK|wxCENTRE|wxICON_EXCLAMATION)
		dialog.ShowModal()
		
		retValue = 0  # error
	else:
		# success!
		# determine if we need layout
		for node in mothership.Nodes():
			(x, y) = node.GetPosition()
			if x == 0 and y == 0:
				mothership.LayoutNodes()
				break
		#endfor
		retValue = 1  # OK

	# restore original sys.argv list
	sys.argv = origArgv

	return retValue

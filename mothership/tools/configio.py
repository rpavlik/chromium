# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

"""Functions for reading and writing Chromium config files."""

import re, string, sys
sys.path.append("../server")
import crtypes, crutils
from crconfig import *


#----------------------------------------------------------------------
# Config file writing


__ConfigFileHeader = """
import string
import sys
sys.path.append( "../server" )
from mothership import *

# Get program name
if len(sys.argv) == 1:
	program = GLOBAL_default_app
elif len(sys.argv) == 2:
	program = sys.argv[1]
else:
	print "Usage: %s <program>" % sys.argv[0] 
	sys.exit(-1)
if program == "":
	print "No program to run!"
	sys.exit(-1)


cr = CR()
cr.MTU( GLOBAL_MTU )

"""

__ConfigFileTail = """
cr.SetParam('minimum_window_size', GLOBAL_minimum_window_size)
cr.SetParam('match_window_title', GLOBAL_match_window_title)
cr.SetParam('show_cursor', GLOBAL_show_cursor)
cr.Go()
"""

def __WriteOption(name, type, values, file):
	"""Helper function"""
	if len(values) == 1:
		valueStr = str(values[0])
	else:
		valueStr = str(values)
	if type == "INT" or type == "BOOL":
		file.write("%s = %s\n" % (name, valueStr))
	elif type == "FLOAT":
		file.write("%s = %s\n" % (pname, valueStr))
	elif type == "STRING":
		file.write("%s = \"%s\"\n" % (name, valueStr))
	else:
		assert type == "LABEL"
		pass

def WriteGlobalOptions(mothership, file):
	for (name, description, type, count, default, mins, maxs) in mothership.GlobalOptions:
		values = mothership.GetGlobalOption(name)
		__WriteOption("GLOBAL_" + name, type, values, file)


def WriteServerOptions(mothership, file):
	for (name, description, type, count, default, mins, maxs) in mothership.ServerOptions:
		values = mothership.GetServerOption(name)
		__WriteOption("SERVER_" + name, type, values, file)


def WriteSPUOptions(spu, prefix, file):
	"""Write SPU options to given file handle."""
	(params, options) = crutils.GetSPUOptions(spu.Name())
	values = {}
	for (name, description, type, count, default, mins, maxs) in options:
		values = spu.GetOption(name)
		__WriteOption(prefix + "_" + name, type, values, file)

def WriteSPUConfs(spu, lvalue, file):
	"""Write a spu.Conf() line to the file handle for all SPU options."""
	(params, options) = crutils.GetSPUOptions(spu.Name())
	values = {}
	for (name, description, type, count, default, mins, maxs) in options:
		values = spu.GetOption(name)
		if values != default:
			if type == "STRING":
				assert count == 1
				file.write("%s('%s', '%s')\n" % (lvalue, name, values[0]))
			else:
				valueStr = ""
				for val in values:
					valueStr += "%s " % str(val)
				# remove last space
				valueStr = valueStr[:-1]
				file.write("%s('%s', '%s')\n" % (lvalue, name, valueStr))
	


def WriteConfig(mothership, file):
	"""Write the mothership config to file handle."""

	file.write("# Chromium configuration produced by graph.py\n")
	# write server and global options
	WriteServerOptions(mothership, file)
	WriteGlobalOptions(mothership, file)
	file.write("\n")
	# boilerplate
	file.write(__ConfigFileHeader)

	# Assign an index to each node (needed for AddServer)
	numNodes = 0
	for node in mothership.Nodes():
		node.index = numNodes
		numNodes += node.GetCount()

	# "declare" the nodes array
	file.write("nodes = range(%d)\n" % numNodes)

	# write the code to allocate the nodes
	i = 0
	for node in mothership.Nodes():
		# emit N nodes
		for j in range(node.GetCount()):
			if node.IsServer():
				file.write("nodes[%d] = CRNetworkNode('%s')\n" %
						   (i, node.GetHosts()[j]))
			else:
				# application node
				file.write("nodes[%d] = CRApplicationNode('%s')\n" %
						   (i, node.GetHosts()[j]))
				file.write("nodes[%d].StartDir( GLOBAL_default_dir )\n" % i)
				file.write("nodes[%d].SetApplication( program )\n" %i)
			file.write("cr.AddNode(nodes[%d])\n" % i)
			i += 1
		#endif
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
					WriteSPUConfs(spu, "spus[%d].Conf" % k, file)
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


#----------------------------------------------------------------------

def ParseOption(s, prefix):
	"""Parsing helper function"""
	# s will be a string like:  RENDER_system_gl_path = "/usr/lib"
	# We'll return a (name, value) tuple like ("system_gl_path", ["/usr/lib"])
	# The name is a string and the value is a list.

	# extract the option name and value
	# parentheses in the regexp define groups
	# \"? is an optional double-quote character
	# [^\"] is any character but double-quote
	varNamePat = "[\w\_]+"    # \w = alphanumeric
	quotedPat = '\"?[^\"]*\"?'   # string in optional double-quotes
	pattern = "^" + prefix + "_(" + varNamePat + ") = (" + quotedPat + ")$"
	v = re.search(pattern, s)
	if v:
		name = v.group(1)
		value = v.group(2)
		if value[0] != '[':
			value = '[' + value + ']'
		values = eval(value)
		return (name, values)
	else:
		print "PROBLEM: " + pattern
		print "LINE: " + s
		return 0


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


def ReadConfig(mothership, file):
	"""Read a mothership config from file handle."""
	global TargetMothership
	TargetMothership = mothership
	print "Begin reading config file..."

	# read entire file into a string
	contents = file.read(-1)

	# Remove some lines which cause trouble
	skipPatterns = [ "^sys\.path\.append.*$",
					 "^from mothership import.*$" ]
	for pat in skipPatterns:
		v = re.search(pat, contents, flags=re.MULTILINE)
		if v:
			print "Ignoring line: %s" % contents[v.start() : v.end()]
			# cut out the offending line
			contents = contents[0 : v.start()] + contents[v.end()+1 : ]
			
	# XXX need a way to diagnose/report errors here
	if 1:
		# catch exceptions
		try:
			exec contents
		except:
			result = "Error"
		else:
			result = "OK"
	else:
		# don't catch exceptions - for debugging
		exec contents
		result = "OK"

	print "Done reading config file: %s" % result

	mothership.LayoutNodes()
	return result



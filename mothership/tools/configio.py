# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

"""Functions for reading and writing Chromium config files."""

import re, string
import crutils


__ConfigFileHeader = """
import string
import sys
sys.path.append( "../server" )
from mothership import *

cr = CR()

"""

__ConfigFileTail = """
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
		if node.IsServer():
			type = "crNetworkNode"
		else:
			type = "crApplicationNode"
		# emit N nodes
		for j in range(node.GetCount()):
			file.write("nodes[%d] = %s('%s')\n" %
					   (i, type, node.GetHosts()[j]))
			file.write("cr.AddNode(nodes[%d])\n" % i)
			i += 1
		#endif
	#endfor
	file.write("\n")

	# write the SPUs for each node
	for node in mothership.Nodes():
		for j in range(node.GetCount()):
			numSPUs = len(node.SPUChain())
			if numSPUs > 0:
				if node.IsServer():
					type = "crNetworkNode"
				else:
					type = "crApplicationNode"
				file.write("# %s nodes[%d]\n" % (type, node.index + j))
				file.write("spus = range(%d)\n" % numSPUs)
				k = 0
				for spu in node.SPUChain():
					file.write("spus[%d] = SPU('%s')\n" % (k, spu.Name()))
					WriteSPUConfs(spu, "spus[%d].Conf" % k, file)
					if k + 1 == numSPUs:
						# last SPU, add servers, if any
						for server in node.GetServers():
							file.write("spus[%d].AddServer(nodes[%d])\n" %
									   (k, server.index))
					file.write("nodes[%d].AddSPU(spu[%d])\n" %
							   (node.index + j, k))
					k += 1
				file.write("\n")
			#endif
		#endfor
		if node.IsServer():
			# write the tiles information
			for j in range(node.GetCount()):
				# XXX fix this
				#file.write("nodes[%d].AddTile(x, y, width, height)\n" %
				#		   node.index + j)
				pass
		#endif
	#endfor

	file.write("\n")

	# tail of file
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


def ReadConfig(mothership, file):
	"""Read a mothership config from file handle."""
	pass



# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

"""Functions for reading and writing Chromium config files."""

import crutils


__ConfigFileHeader = """
import string
import sys
sys.path.append( "../server" )
sys.path.append( "../tools" )
from mothership import *
from crutils import *

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
	else:
		file.write("%s = \"%s\"\n" % (name, valueStr))
	

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


def ReadConfig(mothership, file):
	"""Read a mothership config from file handle."""
	pass



def WriteConfig(mothership, file):
	"""Write the mothership config to file handle."""

	file.write("# Chromium configuration produced by graph.py\n")
	file.write(__ConfigFileHeader)

	# write the nodes and SPUs
	nodeNames = {}
	spuNames = {}
	n = 0
	s = 0
	for node in mothership.Nodes():
		nodeNames[node] = "node[%d]" % n
		if node.IsServer():
			file.write("node[%d] = crNetworkNode('%s')\n" %
					   (n, node.GetHost()[0])) # XXX fix hostnames
		else:
			file.write("node[%d] = crApplicationNode('%s')\n" %
					   (n, node.GetHost()[0]))
		# write the node's SPUs
		for spu in node.SPUChain():
			spuNames[spu] = "spu[%d]" % s
			file.write("spu[%d] = SPU('%s')\n" % (s, spu.Name()))
			file.write("node[%d].AddSPU(spu[%d])\n" % (n, s))
			file.write("#write spu options here\n")
			s += 1
		n += 1
		file.write("\n")

	# add servers to tilesort/packer SPUs
	for node in mothership.Nodes():
		lastSPU = node.LastSPU()
		if lastSPU:
			for server in lastSPU.GetServers():
				file.write("%s.AddServer(%s)\n" % (spuNames[lastSPU],
												   nodeNames[server]))

	f.write("\n")
	# add nodes to mothership
	for node in self.mothership.Nodes():
		f.write("cr.AddNode(%s)\n" % nodeNames[node])

	# tail of file
	f.write(__ConfigFileTail)


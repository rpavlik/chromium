# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

"""Functions for reading and writing Chromium config files."""


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



def WriteConfig(mothership, file):
	"""Write the mothership config to file handle."""
	pass



def ReadConfig(mothership, file):
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
					   (n, node.GetHost()))
		else:
			file.write("node[%d] = crApplicationNode('%s')\n" %
					   (n, node.GetHost()))
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


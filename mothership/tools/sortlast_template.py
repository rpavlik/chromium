# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

""" sortlast_template.py
    Sort-last template module.
"""

import string, cPickle, os.path, re
from wxPython.wx import *
import traceback, types
import intdialog, spudialog, hostdialog
import crutils, crtypes, configio


def Create_Sortlast(parentWindow, mothership):
	"""Create a sort-last configuration"""
	dialog = intdialog.IntDialog(NULL, id=-1,
								 title="Sort-last Template",
								 labels=["Number of application nodes:",
										 "Window Width:",
										 "Window Height:"],
								 defaultValues=[2, 512, 512], maxValue=10000)
	if dialog.ShowModal() == wxID_CANCEL:
		dialog.Destroy()
		return 0
	numClients = dialog.GetValue()
	mothership.DeselectAllNodes()

	# Create the server/render node
	xPos = 300
	if 1:
		yPos = 5
	else:
		yPos = numClients * 60 / 2 - 20
	hosts = crutils.GetSiteDefault("client_hosts")
	if not hosts:
		hosts = ["localhost"]
	serverNode = crtypes.NetworkNode(hosts, 1)
	serverNode.SetPosition(xPos, yPos)
	serverNode.Select()
	renderSPU = crutils.NewSPU("render")
	serverNode.AddSPU(renderSPU)
	mothership.AddNode(serverNode)

	# Create the client/app nodes
	xPos = 5
	yPos = 5
	hosts = crutils.GetSiteDefault("server_hosts")
	if not hosts:
		hosts = ["localhost"]
	appNode = crtypes.ApplicationNode(hosts, numClients)
	appNode.SetPosition(xPos, yPos)
	appNode.Select()
	readbackSPU = crutils.NewSPU("readback")
	appNode.AddSPU(readbackSPU)
	packSPU = crutils.NewSPU("pack")
	appNode.AddSPU(packSPU)
	mothership.AddNode(appNode)
	packSPU.AddServer(serverNode)
	dialog.Destroy()
	return 1


def Is_Sortlast(mothership):
	"""Test if the mothership describes a sort-last configuration.
	Return 1 if so, 0 otherwise."""
	# First, check for correct number and type of nodes
	nodes = mothership.Nodes()
	if len(nodes) != 2: 
		print "not 2 nodes"
		return 0
	if not ((nodes[0].IsAppNode() and nodes[1].IsServer()) or
			(nodes[1].IsAppNode() and nodes[0].IsServer())):
		# one node must be the app node, the other the server
		print "bad nodes"
		return 0
	# Find app/server nodes
	if nodes[0].IsAppNode():
		appNode = nodes[0]
		serverNode = nodes[1]
	else:
		appNode = nodes[1]
		serverNode = nodes[0]
	# Find SPUs
	renderSPU = serverNode.LastSPU()
	if renderSPU.Name() != "render":
		print "no render SPU"
		return 0
	readbackSPU = appNode.SPUChain()[0]
	if readbackSPU.Name() != "readback":
		print "no readback SPU"
		return 0
	packSPU = appNode.SPUChain()[1]
	if packSPU.Name() != "pack":
		print "no pack SPU"
		return 0
	# Next, check that the app's servers are correct
	servers = packSPU.GetServers()
	if len(servers) != 1 or servers[0] != serverNode:
		print "no client/server connection"
		return 0
	# OK, this is a sort-last config!
	return 1


def Edit_Sortlast(parentWindow, mothership):
	"""Edit parameters for a sort-last template"""
	# widgets:
	# Button for Readback SPU ...
	# Button for Render SPU ...
	# application name with #rank, #size, #zeroth substitution
	pass


def Read_Sortlast(mothership, file):
	pass


def Write_Sortlast(mothership, file):
	pass


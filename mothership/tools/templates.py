# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

"""Functions for Chromium configuration templates"""

from wxPython.wx import *
import crtypes
import crutils
import intdialog
import tilesort_template
import sortlast_template


def __Create_BinarySwap(parentWindow, mothership):
	"""Create a binary-swap, sort-last configuration"""
	# XXX need an integer dialog here!!!!
	# XXX also, a widget for the hostnames???
	dialog = intdialog.IntDialog(NULL, id=-1,
								 title="Binary-swap Template",
								 labels=["Number of application nodes:"],
								 defaultValues=[2], maxValue=10000)
	if dialog.ShowModal() == wxID_CANCEL:
		dialog.Destroy()
		return 0
	numClients = dialog.GetValue()
	hostname = "localhost"
	mothership.DeselectAllNodes()
	# Create the server/render node
	xPos = 300
	if 1:
		yPos = 5
	else:
		yPos = numClients * 60 / 2 - 20
	serverNode = crutils.NewNetworkNode(1)
	serverNode.SetPosition(xPos, yPos)
	serverNode.Select()
	renderSPU = crutils.NewSPU("render")
	serverNode.AddSPU(renderSPU)
	mothership.AddNode(serverNode)
	# Create the client/app nodes
	xPos = 5
	yPos = 5
	if 1:
		appNode = crutils.NewApplicationNode(numClients)
		appNode.SetPosition(xPos, yPos)
		appNode.Select()
		readbackSPU = crutils.NewSPU("binaryswap")
		appNode.AddSPU(readbackSPU)
		packSPU = crutils.NewSPU("pack")
		appNode.AddSPU(packSPU)
		mothership.AddNode(appNode)
		packSPU.AddServer(serverNode)
	else:
		for i in range(0, numClients):
			appNode = crutils.NewApplicationNode(1)
			appNode.SetPosition(xPos, yPos)
			appNode.Select()
			readbackSPU = crutils.NewSPU("binaryswap")
			appNode.AddSPU(readbackSPU)
			packSPU = crutils.NewSPU("pack")
			appNode.AddSPU(packSPU)
			mothership.AddNode(appNode)
			packSPU.AddServer(serverNode)
			yPos += 60
	dialog.Destroy()
	return 1

def __Is_BinarySwap(mothership):
	# XXX temporary
	return 1

def __Edit_BinarySwap(parentWindow, mothership):
	"""Edit parameters for a Tilesort template"""
	pass

def __Read_BinarySwap(mothership, file):
	pass

def __Write_BinarySwap(mothership, file):
	pass


# XXX Eventually we'll scan a template directory to do build a list
# of templates dynamically.
# XXX also specify functions for editing options, saving, validation, etc.

__Templates = {
	"Tilesort"     : ( tilesort_template.Create_Tilesort,
					   tilesort_template.Is_Tilesort,
					   tilesort_template.Edit_Tilesort,
					   tilesort_template.Read_Tilesort,
					   tilesort_template.Write_Tilesort ),
	"Sort-last"    : ( sortlast_template.Create_Sortlast,
					   sortlast_template.Is_Sortlast,
					   sortlast_template.Edit_Sortlast,
					   sortlast_template.Read_Sortlast,
					   sortlast_template.Write_Sortlast ),
	"Binary-swap"  : ( __Create_BinarySwap,
					   __Is_BinarySwap,
					   __Edit_BinarySwap,
					   __Read_BinarySwap,
					   __Write_BinarySwap ),
}

def GetTemplateList():
	"""Return list of names of known templates."""
	return __Templates.keys()


def CreateTemplate(templateName, parentWindow, mothership):
	"""Create an instance of the named template."""
	assert templateName in __Templates.keys()
	create = __Templates[templateName][0]
	assert create
	if create(parentWindow, mothership):
		mothership.SetTemplateType(templateName)
	else:
		mothership.SetTemplateType("")


def ValidateTemplate(templateName, mothership):
	"""Determine if the mothership matches the given template type."""
	assert templateName in __Templates.keys()
	(create, validate, edit, read, write) = __Templates[templateName]
	assert validate
	return validate(mothership)


def EditTemplate(templateName, parentWindow, mothership):
	"""Edit a templatized configuration."""
	assert templateName in __Templates.keys()
	(create, validate, edit, read, write) = __Templates[templateName]
	assert validate
	assert edit
	if validate(mothership):
		edit(parentWindow, mothership)
	else:
		# Inform the user that there's a problem
		dialog = wxMessageDialog(parent=parentWindow,
						 message="This configuration doesn't appear to be a "
						 + templateName + " configuration.",
						 caption="Notice", style=wxOK)
		dialog.ShowModal()
		#mothership.SetTemplateType("")


def ReadTemplate(templateName, mothership, file):
	"""Read a templatized mothership config from the given file handle."""
	assert templateName in __Templates.keys()
	(create, validate, edit, read, write) = __Templates[templateName]
	if read(mothership, file):
		mothership.SetTemplateType(templateName)
		assert validate(mothership)
		return 1
	else:
		return 0


def WriteTemplate(templateName, mothership, file):
	"""Write a templatized mothership config to given file handle."""
	assert templateName in __Templates.keys()
	(create, validate, edit, read, write) = __Templates[templateName]
	assert validate(mothership)
	write(mothership, file)

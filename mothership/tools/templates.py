"""Functions for Chromium configuration templates"""

from wxPython.wx import *
import crtypes
import crutils
import intdialog
from tilesort_temp import *


# Eventually we'll move these into separate files in a special directory.
# There'll be separate functions/interfaces for instantiating and editing
# these templates/assemblies.

def __Create_Tilesort(parentWindow, mothership):
	"""Create a tilesort configuration"""
	# XXX need a widget for the hostnames???
	dialog = intdialog.IntDialog(NULL, id=-1,
								 title="Tilesort Template",
								 labels=["Number of client/application nodes:",
										 "Number of server/render nodes:"],
								 defaultValues=[1, 4], maxValue=10000)
	if dialog.ShowModal() == wxID_CANCEL:
		dialog.Destroy()
		return 0
	values = dialog.GetValues()
	numClients = values[0]
	numServers = values[1]
	m = max(numClients, numServers)
	hostname = "localhost"
	mothership.DeselectAllNodes()
	# Create the <numClients> app nodes
	appNode = crtypes.ApplicationNode(host=hostname)
	appNode.SetPosition(50, 50)
	appNode.SetCount(numClients)
	appNode.Select()
	tilesortSPU = crutils.NewSPU("tilesort")
	appNode.AddSPU(tilesortSPU)
	mothership.AddNode(appNode)
	# Create the <numServers> server nodes
	serverNode = crtypes.NetworkNode(host=hostname)
	serverNode.SetPosition(350, 50)
	serverNode.SetCount(numServers)
	serverNode.Select()
	renderSPU = crutils.NewSPU("render")
	serverNode.AddSPU(renderSPU)
	mothership.AddNode(serverNode)
	tilesortSPU.AddServer(serverNode)
	# All done
	dialog.Destroy()
	return 1


def __Edit_Tilesort(parentWindow, mothership):
	"""Edit parameters for a Tilesort template"""
	d = TilesortFrame()
	d.Centre()
	d.Show(TRUE)
	print "Edit tilesort..."
	pass


def __Create_Sortlast(parentWindow, mothership):
	"""Create a sort-last configuration"""
	# XXX need a widget for the hostnames???
	dialog = intdialog.IntDialog(NULL, id=-1,
								 title="Sort-last Template",
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
	serverNode = crtypes.NetworkNode(host=hostname)
	serverNode.SetPosition(xPos, yPos)
	serverNode.Select()
	renderSPU = crutils.NewSPU("render")
	serverNode.AddSPU(renderSPU)
	mothership.AddNode(serverNode)
	# Create the client/app nodes
	xPos = 5
	yPos = 5
	if 1:
		# Make one N-count node
		appNode = crtypes.ApplicationNode(host=hostname)
		appNode.SetPosition(xPos, yPos)
		appNode.SetCount(numClients)
		appNode.Select()
		readbackSPU = crutils.NewSPU("readback")
		appNode.AddSPU(readbackSPU)
		packSPU = crutils.NewSPU("pack")
		appNode.AddSPU(packSPU)
		mothership.AddNode(appNode)
		packSPU.AddServer(serverNode)
	else:
		# Make N discreet nodes
		for i in range(0, numClients):
			appNode = crtypes.ApplicationNode(host=hostname)
			appNode.SetPosition(xPos, yPos)
			appNode.Select()
			readbackSPU = crutils.NewSPU("readback")
			appNode.AddSPU(readbackSPU)
			packSPU = crutils.NewSPU("pack")
			appNode.AddSPU(packSPU)
			mothership.AddNode(appNode)
			packSPU.AddServer(serverNode)
			yPos += 60
	dialog.Destroy()
	return 1

def __Edit_Sortlast(parentWindow, mothership):
	"""Edit parameters for a sort-last template"""
	pass


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
	serverNode = crtypes.NetworkNode(host=hostname)
	serverNode.SetPosition(xPos, yPos)
	serverNode.Select()
	renderSPU = crutils.NewSPU("render")
	serverNode.AddSPU(renderSPU)
	mothership.AddNode(serverNode)
	# Create the client/app nodes
	xPos = 5
	yPos = 5
	if 1:
		appNode = crtypes.ApplicationNode(host=hostname)
		appNode.SetPosition(xPos, yPos)
		appNode.SetCount(numClients)
		appNode.Select()
		readbackSPU = crutils.NewSPU("binaryswap")
		appNode.AddSPU(readbackSPU)
		packSPU = crutils.NewSPU("pack")
		appNode.AddSPU(packSPU)
		mothership.AddNode(appNode)
		packSPU.AddServer(serverNode)
	else:
		for i in range(0, numClients):
			appNode = crtypes.ApplicationNode(host=hostname)
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

def __Edit_BinarySwap(parentWindow, mothership):
	"""Edit parameters for a Tilesort template"""
	pass


# XXX Eventually we'll scan a template directory to do build a list
# of templates dynamically.
# XXX also specify functions for editing options, saving, validation, etc.

__Templates = {
	"Tilesort"     : (__Create_Tilesort, __Edit_Tilesort),
	"Sort-last"    : (__Create_Sortlast, __Edit_Sortlast),
	"Binary-swap"  : (__Create_BinarySwap, __Edit_BinarySwap),
#	"Lightning-2"  : (__Create_Lightning2, __Edit_Lightning2)
}

def GetTemplateList():
	"""Return a list of known templates."""
	return __Templates.keys()


def CreateTemplate(templateName, parentWindow, mothership):
	"""Create an instance of the named template."""
	assert templateName in __Templates.keys()
	create = __Templates[templateName][0]
	assert create
	if create(parentWindow, mothership):
		mothership.SetTemplateType(templateName)


def EditTemplate(templateName, parentWindow, mothership):
	"""Edit a templatized configuration."""
	assert templateName in __Templates.keys()
	edit = __Templates[templateName][1]
	assert edit
	edit(parentWindow, mothership)

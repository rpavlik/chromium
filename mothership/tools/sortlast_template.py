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


class SortlastParameters:
	"""C-style struct describing a sortlast configuration"""
	# This is where we set all the default sortlast parameters.
	def __init__(self):
		self.ZerothArg = ''
		self.NthArg = ''

	def Clone(self):
		"""Return a clone of this object."""
		# We're not using the copy.copy() function since it's flakey
		p = SortlastParameters()
		p.ZerothArg = self.ZerothArg
		p.NthArg = self.NthArg
		return p


DefaultWidth = 512
DefaultHeight = 512


#----------------------------------------------------------------------

# This is the guts of the sortlast configuration script.
# It's simply appended to the file after we write all the configuration options
__ConfigBody = """
import string, sys
sys.path.append( "../server" )
from mothership import *

# Check for program name/args on command line
if len(sys.argv) == 1:
	program = GLOBAL_default_app
else:
	program = string.join(sys.argv[1:])
if program == "":
	print "No program to run!"
	sys.exit(-1)

cr = CR()
cr.MTU( GLOBAL_MTU )

# Make the server/compositing node
serverNode = CRNetworkNode( COMPOSITE_HOST )
renderSPU = SPU( 'render' )
renderSPU.Conf( 'window_geometry',
				RENDER_window_geometry[0],
				RENDER_window_geometry[1],
				RENDER_window_geometry[2],
				RENDER_window_geometry[3] )
renderSPU.Conf( 'try_direct', RENDER_try_direct )
renderSPU.Conf( 'force_direct', RENDER_force_direct )
renderSPU.Conf( 'fullscreen', RENDER_fullscreen )
renderSPU.Conf( 'title', RENDER_title )
renderSPU.Conf( 'system_gl_path', RENDER_system_gl_path )

serverNode.AddSPU( renderSPU )
cr.AddNode( serverNode )

# Make the client/app nodes
readbackSPUs = []
appNodes = []

for i in range(NUM_APP_NODES):
	node = CRApplicationNode( APP_HOSTS[i] )
	# argument substitutions
	if i == 0 and ZEROTH_ARG != "":
		app_string = string.replace( program, '#zeroth', ZEROTH_ARG)
	else:
		app_string = string.replace( program, '#zeroth', '' )
	app_string = string.replace( app_string, '#nth', str(i) )
	node.SetApplication( app_string )
	node.StartDir( GLOBAL_default_dir )

	readbackSPU = SPU( 'readback' )
	readbackSPU.Conf( 'window_geometry',
						READBACK_window_geometry[0],
						READBACK_window_geometry[1],
						READBACK_window_geometry[2],
						READBACK_window_geometry[3] )
	readbackSPU.Conf( 'try_direct', READBACK_try_direct )
	readbackSPU.Conf( 'force_direct', READBACK_force_direct )
	readbackSPU.Conf( 'fullscreen', READBACK_fullscreen )
	readbackSPU.Conf( 'title', READBACK_title )
	readbackSPU.Conf( 'system_gl_path', READBACK_system_gl_path )
	readbackSPU.Conf( 'extract_depth', READBACK_extract_depth )
	readbackSPU.Conf( 'extract_alpha', READBACK_extract_alpha )
	readbackSPU.Conf( 'local_visualization', READBACK_local_visualization )
	readbackSPU.Conf( 'visualize_depth', READBACK_visualize_depth )
	readbackSPU.Conf( 'drawpixels_pos', READBACK_drawpixels_pos[0],
						READBACK_drawpixels_pos[1] )
	node.AddSPU( readbackSPU )
	packSPU = SPU( 'pack' )
	node.AddSPU( packSPU )
	packSPU.AddServer( serverNode, 'tcpip' )

	appNodes.append( node )
	readbackSPUs.append( readbackSPU )
	cr.AddNode( node )

	# XXX do auto_run code!

# Run mothership
cr.Go()

"""

#----------------------------------------------------------------------


def FindClientNode(mothership):
	"""Search the mothership for the client node."""
	nodes = mothership.Nodes()
	assert len(nodes) == 2
	if nodes[0].IsAppNode():
		return nodes[0]
	else:
		assert nodes[1].IsAppNode()
		return nodes[1]

def FindServerNode(mothership):
	"""Search the mothership for the server node."""
	nodes = mothership.Nodes()
	assert len(nodes) == 2
	if nodes[0].IsServer():
		return nodes[0]
	else:
		assert nodes[1].IsServer()
		return nodes[1]

def FindReadbackSPU(mothership):
	"""Search the mothership for the readback SPU."""
	appNode = FindClientNode(mothership)
	readbackSPU = appNode.SPUChain()[0]
	assert readbackSPU.Name() == "readback"
	return readbackSPU

def FindRenderSPU(mothership):
	"""Search the mothership for the render SPU."""
	serverNode = FindServerNode(mothership)
	renderSPU = serverNode.LastSPU()
	assert renderSPU.Name() == "render"
	return renderSPU


#----------------------------------------------------------------------

class SortlastDialog(wxDialog):
	"""Sortlast configuration editor."""

	def __init__(self, parent=NULL, id=-1):
		""" Construct a SortlastDialog."""
		wxDialog.__init__(self, parent, id, title="Sort-last Configuration",
						 style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)

		# Widget IDs
		id_Width           = 4000
		id_Height          = 4001
		id_NumApps         = 4002
		id_ReadbackOptions = 4003
		id_RenderOptions   = 4004
		id_Hostnames       = 4005
		id_OK              = 4011
		id_CANCEL          = 4012

		# init misc member vars
		self.__Mothership = 0
		self.dirty = false

		# this sizer holds all the sortlast control widgets
		outerSizer = wxBoxSizer(wxVERTICAL)

		# Window width/height (in pixels)
		box = wxStaticBox(parent=self, id=-1,
						  label="Window Size (updates all SPUs)",
						  style=wxDOUBLE_BORDER)
		windowSizeSizer = wxStaticBoxSizer(box, wxHORIZONTAL)
		widthLabel = wxStaticText(parent=self, id=-1, label="Width:")
		self.widthControl = wxSpinCtrl(parent=self, id=id_Width,
									   value="1", min=1, max=2048,
									   size=wxSize(70,25))
		heightLabel = wxStaticText(parent=self, id=-1, label="Height:")
		self.heightControl = wxSpinCtrl(parent=self, id=id_Height,
									   value="1", min=1, max=2048,
									   size=wxSize(70,25))
		EVT_SPINCTRL(self.widthControl, id_Width, self.__OnSizeChange)
		EVT_SPINCTRL(self.heightControl, id_Height, self.__OnSizeChange)
		windowSizeSizer.Add(widthLabel,
							flag=wxALIGN_CENTER_VERTICAL|wxALL, border=4)
		windowSizeSizer.Add(self.widthControl,
							flag=wxALIGN_CENTER_VERTICAL|wxALL, border=2)
		windowSizeSizer.Add(heightLabel,
							flag=wxALIGN_CENTER_VERTICAL|wxALL, border=4)
		windowSizeSizer.Add(self.heightControl,
							flag=wxALIGN_CENTER_VERTICAL|wxALL, border=2)
		# XXX add drop-down widget with common sizes
		outerSizer.Add(windowSizeSizer, flag=wxALL|wxGROW, border=4)

		# Number of app nodes
		box = wxStaticBox(parent=self, id=-1, label="Application Nodes",
						  style=wxDOUBLE_BORDER)
		appSizer = wxStaticBoxSizer(box, wxHORIZONTAL)
		numberLabel = wxStaticText(parent=self, id=-1, label="Number:")
		self.numberControl = wxSpinCtrl(parent=self, id=id_Width,
										value="1", min=1, max=10000,
										size=wxSize(70,25))
		EVT_SPINCTRL(self.numberControl, id_NumApps, self.__OnNumAppsChange)
		appSizer.Add(numberLabel, flag=wxALIGN_CENTER_VERTICAL|wxALL, border=4)
		appSizer.Add(self.numberControl,
					 flag=wxALIGN_CENTER_VERTICAL|wxALL, border=2)
		self.hostsButton = wxButton(parent=self, id=id_Hostnames,
									label=" Host Names... ")
		appSizer.Add(self.hostsButton, flag=wxALL, border=2)
		EVT_BUTTON(self.hostsButton, id_Hostnames, self.__OnHostnames)
		outerSizer.Add(appSizer, flag=wxALL|wxGROW, border=4)

		# SPU options box
		box = wxStaticBox(parent=self, id=-1, label="SPU Options",
						  style=wxDOUBLE_BORDER)
		spuSizer = wxStaticBoxSizer(box, wxHORIZONTAL)
		self.readbackButton = wxButton(parent=self, id=id_ReadbackOptions,
									   label=" Readback... ")
		spuSizer.Add(self.readbackButton,
					  flag=wxALL|wxALIGN_CENTRE_HORIZONTAL, border=2)
		EVT_BUTTON(self.readbackButton, id_ReadbackOptions,
				   self.__OnReadbackOptions)
		self.renderButton = wxButton(parent=self, id=id_RenderOptions,
									   label=" Render... ")
		spuSizer.Add(self.renderButton,
					 flag=wxALL|wxALIGN_CENTRE_HORIZONTAL, border=2)
		EVT_BUTTON(self.renderButton, id_RenderOptions,
				   self.__OnRenderOptions)
		outerSizer.Add(spuSizer, flag=wxALL|wxGROW, border=4)

		# Sizer for the OK, Cancel buttons
		okCancelSizer = wxGridSizer(rows=1, cols=2, vgap=4, hgap=20)
		self.OkButton = wxButton(parent=self, id=id_OK, label="OK")
		okCancelSizer.Add(self.OkButton, option=0,
						  flag=wxALIGN_CENTER, border=0)
		self.CancelButton = wxButton(parent=self, id=id_CANCEL,
									 label="Cancel")
		okCancelSizer.Add(self.CancelButton, option=0,
						  flag=wxALIGN_CENTER, border=0)
		EVT_BUTTON(self.OkButton, id_OK, self._onOK)
		EVT_BUTTON(self.CancelButton, id_CANCEL, self._onCancel)

		# horizontal separator (box with height=0)
		separator = wxStaticBox(parent=self, id=-1,
								label="", size=wxSize(10,0))

		# outer-most sizer
		outerSizer.Add(separator, flag=wxGROW|wxALL, border=4)
		outerSizer.Add(okCancelSizer, option=0, flag=wxALL|wxGROW, border=10)

		self.SetAutoLayout(true)
		self.SetSizer(outerSizer)

		minSize = outerSizer.GetMinSize()
		self.SetSizeHints(minW=minSize[0], minH=minSize[1])
		self.SetSize(minSize)

		# Hostname dialog
		self.hostsDialog = hostdialog.HostDialog(parent=NULL, id=-1,
						title="Chromium Hosts",
						message="Specify host names for the readback nodes")

	# end of __init__()

	def __UpdateVarsFromWidgets(self):
		"""Get current widget values and update the sortlast parameters."""
		# XXX do window width, height
		#tilesort = self.__Mothership.Tilesort
		#tilesort.Columns = self.widthControl.GetValue()
		#tilesort.Rows = self.heightControl.GetValue()
		#tilesort.TileWidth = self.tileWidthControl.GetValue()
		#tilesort.TileHeight = self.tileHeightControl.GetValue()
		#tilesort.RightToLeft = self.hLayoutRadio.GetSelection()
		#tilesort.BottomToTop = self.vLayoutRadio.GetSelection()

	def __UpdateWidgetsFromVars(self):
		"""Set widget values to the tilesort parameters."""
		# XXX do window width, height
		#tilesort = self.__Mothership.Tilesort
		#self.widthControl.SetValue(tilesort.Columns)
		#self.heightControl.SetValue(tilesort.Rows)
		#self.tileWidthControl.SetValue(tilesort.TileWidth)
		#self.tileHeightControl.SetValue(tilesort.TileHeight)
		#self.hLayoutRadio.SetSelection(tilesort.RightToLeft)
		#self.vLayoutRadio.SetSelection(tilesort.BottomToTop)

	# ----------------------------------------------------------------------
	# Event handling

	def __OnSizeChange(self, event):
		"""Called when window size changes with spin controls."""
		self.__UpdateVarsFromWidgets()
		self.dirty = true

	def __OnNumAppsChange(self, event):
		"""Called when number of app nodes spin control changes."""
		#self.__UpdateVarsFromWidgets()
		self.dirty = true

	def __OnHostnames(self, event):
		"""Called when the hostnames button is pressed."""
		sortlast = self.__Mothership.Sortlast
		clientNode = FindClientNode(self.__Mothership)
		self.hostsDialog.SetHostPattern(clientNode.GetHostNamePattern())
		self.hostsDialog.SetCount(clientNode.GetCount())
		self.hostsDialog.SetHosts(clientNode.GetHosts())
		if self.hostsDialog.ShowModal() == wxID_OK:
			clientNode.SetHostNamePattern(self.hostsDialog.GetHostPattern())
			clientNode.SetHosts(self.hostsDialog.GetHosts())

	def __OnReadbackOptions(self, event):
		"""Called when Readback SPU Options button is pressed."""
		readbackSPU = FindReadbackSPU(self.__Mothership)
		(params, opts) = crutils.GetSPUOptions("readback")
		# create the dialog
		dialog = spudialog.SPUDialog(parent=NULL, id=-1,
									 title="Readback SPU Options",
									 options = opts)
		# set the dialog widget values
		dialog.SetValues(readbackSPU.GetOptions())
		# wait for OK or cancel
		if dialog.ShowModal() == wxID_OK:
			# save the new values/options
			readbackSPU.SetOptions(dialog.GetValues())
		else:
			# user cancelled, do nothing, new values are ignored
			pass

	def __OnRenderOptions(self, event):
		"""Called when Render SPU Options button is pressed."""
		renderSPU = FindRenderSPU(self.__Mothership)
		(params, opts) = crutils.GetSPUOptions("render")
		# create the dialog
		dialog = spudialog.SPUDialog(parent=NULL, id=-1,
									 title="Render SPU Options",
									 options = opts)
		# set the dialog widget values
		dialog.SetValues(renderSPU.GetOptions())
		# wait for OK or cancel
		if dialog.ShowModal() == wxID_OK:
			# save the new values/options
			renderSPU.SetOptions(dialog.GetValues())
		else:
			# user cancelled, do nothing, new values are ignored
			pass

	def _onOK(self, event):
		"""Called by OK button"""
		self.EndModal(wxID_OK)

	def _onCancel(self, event):
		"""Called by Cancel button"""
		self.EndModal(wxID_CANCEL)

	def SetMothership(self, mothership):
		"""Specify the mothership to modify.
		mothership is a Mothership object.
		"""
		self.__Mothership = mothership
		# update all the widgets to the template's values
		self.__UpdateWidgetsFromVars()


def Create_Sortlast(parentWindow, mothership):
	"""Create a sort-last configuration"""
	dialog = intdialog.IntDialog(NULL, id=-1,
							 title="Sort-last Template",
							 labels=["Number of application nodes:",
									 "Window Width:",
									 "Window Height:"],
							 defaultValues=[2, DefaultWidth, DefaultHeight],
							 maxValue=10000)
	if dialog.ShowModal() == wxID_CANCEL:
		dialog.Destroy()
		return 0
	numClients = dialog.GetValues()[0]
	width = dialog.GetValues()[1]
	height = dialog.GetValues()[2]

	mothership.Sortlast = SortlastParameters()

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
	renderSPU.SetOption("window_geometry", [0, 0, width, height])
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
	readbackSPU.SetOption("title", ["Chromium Readback SPU"])
	readbackSPU.SetOption("window_geometry", [0, 0, width, height])
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
	# info about command-line substitution

	t = Is_Sortlast(mothership)
	if t:
		clientNode = FindClientNode(mothership)
		# find the server/render nodes
		clientNode = FindClientNode(mothership)
		serverNode = FindServerNode(mothership)
	else:
		print "This is not a sortlast configuration!"
		return

	d = SortlastDialog()
	d.Centre()
	backupSortlastParams = mothership.Sortlast.Clone()
	d.SetMothership(mothership)

	if d.ShowModal() == wxID_CANCEL:
		# restore original values
		mothership.Sortlast = backupSortlastParams
	else:
		# update mothership with new values
		pass

	pass


def Read_Sortlast(mothership, file):
	"""Read a sortlast config from the given file handle."""

	mothership.Sortlast = SortlastParameters()

	serverNode = crtypes.NetworkNode()
	renderSPU = crutils.NewSPU("render")
	serverNode.AddSPU(renderSPU)

	clientNode = crtypes.ApplicationNode()
	readbackSPU = crutils.NewSPU("readback")
	clientNode.AddSPU(readbackSPU)
	packSPU = crutils.NewSPU("pack")
	clientNode.AddSPU(packSPU)
	packSPU.AddServer(serverNode)

	mothership.AddNode(clientNode)
	mothership.AddNode(serverNode)

	numAppNodes = 1

	while true:
		l = file.readline()
		if not l:
			break
		# remove trailing newline character
		if l[-1:] == '\n':
			l = l[:-1]
		if re.match("^COMPOSITE_HOST = ", l):
			v = re.search('\".+\"$', l)
			host = eval(l[v.start() : v.end()])
			serverNode.SetHosts( [ host ] )
		elif re.match("^NUM_APP_NODES = [0-9]+$", l):
			v = re.search("[0-9]+", l)
			clientNode.SetCount( int(l[v.start() : v.end()]) )
		elif re.match("^APP_HOSTS = ", l):
			v = re.search("\[.+\]$", l)
			hosts = eval(l[v.start() : v.end()])
			clientNode.SetHosts( hosts )
		elif re.match("^ZEROTH_ARG = ", l):
			v = re.search('\".+\"$', l)
			mothership.Sortlast.ZerothArg = l[v.start()+1 : v.end()-1]
		elif re.match("^READBACK_", l):
			# A readback SPU option
			(name, values) = configio.ParseOption(l, "READBACK")
			readbackSPU.SetOption(name, values)
		elif re.match("^RENDER_", l):
			# A render SPU option
			(name, values) = configio.ParseOption(l, "RENDER")
			renderSPU.SetOption(name, values)
		elif re.match("^SERVER_", l):
			# A server option
			(name, values) = configio.ParseOption(l, "SERVER")
			mothership.SetServerOption(name, values)
		elif re.match("^GLOBAL_", l):
			# A global option
			(name, values) = configio.ParseOption(l, "GLOBAL")
			mothership.SetGlobalOption(name, values)
		elif re.match("^# end of options", l):
			# that's the end of the variables
			# save the rest of the file....
			break
		elif (l != "") and (not re.match("\s*#", l)):
			print "unrecognized line: %s" % l
	# endwhile

	mothership.LayoutNodes()
	return 1  # OK


def Write_Sortlast(mothership, file):
	"""Write a sort-last config to the file handle."""
	assert Is_Sortlast(mothership)
	assert mothership.GetTemplateType() == "Sort-last"

	print "Writing sort-last config"
	sortlast = mothership.Sortlast
	clientNode = FindClientNode(mothership)
	serverNode = FindServerNode(mothership)

	file.write('TEMPLATE = "Sort-last"\n')
	file.write('COMPOSITE_HOST = "%s"\n' % serverNode.GetHosts()[0])
	file.write('NUM_APP_NODES = %d\n' % clientNode.GetCount())
	file.write('APP_HOSTS = %s\n' % str(clientNode.GetHosts()))
	file.write('ZEROTH_ARG = "%s"\n' % 'fix-me')

	# write render SPU options
	renderSPU = FindRenderSPU(mothership)
	configio.WriteSPUOptions(renderSPU, "RENDER", file)

	# write readback SPU options
	readbackSPU = FindReadbackSPU(mothership)
	configio.WriteSPUOptions(readbackSPU, "READBACK", file)

	# write server and global options
	configio.WriteServerOptions(mothership, file)
	configio.WriteGlobalOptions(mothership, file)

	file.write("# end of options, the rest is boilerplate\n")
	file.write(__ConfigBody)
	return 1  # OK


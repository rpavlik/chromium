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
import intdialog, spudialog, hostdialog, textdialog
import crutils, crtypes, configio
import templatebase
sys.path.append("../server")
import crconfig


class SortlastParameters:
	"""C-style struct describing a sortlast configuration"""
	# This is where we set all the default sortlast parameters.
	# XXX this class will probably go away!  Not really needed.
	def __init__(self):
		self.ZerothArg = ''

	def Clone(self):
		"""Return a clone of this object."""
		# We're not using the copy.copy() function since it's flakey
		p = SortlastParameters()
		p.ZerothArg = self.ZerothArg
		return p


# Predefined window sizes shown in the wxChoice widget (feel free to change)
CommonWindowSizes = [ [128, 128],
					  [256, 256],
					  [512, 512],
					  [1024, 1024],
					  [1280, 1024],
					  [1600, 1200] ]

#----------------------------------------------------------------------

_ImportsSection = """
import string, sys
sys.path.append( "../server" )
from mothership import *

"""

# This is the guts of the sortlast configuration script.
# It's simply appended to the file after we write all the configuration options
_ConfigBody = """
# Look for some special options
for (name, value) in APP_OPTIONS:
	if name == "application":
		DEFAULT_APP = value
	elif name == "zeroth_arg":
		ZEROTH_ARG = value
for (name, value) in MOTHERSHIP_OPTIONS:
	if name == "auto_start":
		AUTO_START = value

# Check for program name/args on command line
if len(sys.argv) == 1:
	program = DEFAULT_APP
else:
	program = string.join(sys.argv[1:])
if program == "":
	print "No program to run!"
	sys.exit(-1)

cr = CR()

# Make the server/compositing node
serverNode = CRNetworkNode( COMPOSITE_HOST )
for (name, value) in SERVER_OPTIONS:
	serverNode.Conf(name, value)
renderSPU = SPU( 'render' )
for (name, value) in RENDER_OPTIONS:
	renderSPU.Conf(name, value)
serverNode.AddSPU( renderSPU )

if AUTO_START:
	serverNode.AutoStart( ["/usr/bin/rsh", COMPOSITE_HOST,
							"/bin/sh -c 'DISPLAY=:0.0  CRMOTHERSHIP=%s  LD_LIBRARY_PATH=%s  crserver'" % (COMPOSITE_HOST, crlibdir) ] )

cr.AddNode( serverNode )


# Make the client/app nodes
readbackSPUs = []
appNodes = []

for i in range(NUM_APP_NODES):
	node = CRApplicationNode( APP_HOSTS[i] )
	for (name, value) in APP_OPTIONS:
		node.Conf(name, value)

	# argument substitutions
	if i == 0 and ZEROTH_ARG != "":
		app_string = string.replace( program, '%0', ZEROTH_ARG)
	else:
		app_string = string.replace( program, '%0', '' )
	app_string = string.replace( app_string, '%I', str(i) )
	app_string = string.replace( app_string, '%N', str(NUM_APP_NODES) )
	node.Conf( 'application', app_string )

	if AUTO_START:
		# XXX this probably doesn't work yet!
		node.AutoStart( ["/usr/bin/rsh", APP_HOSTS[i],
				"/bin/sh -c 'CRMOTHERSHIP=%s LD_LIBRARY_PATH=%s /usr/local/bin/crappfaker'" % (COMPOSITE_HOST, crlibdir)] )

	readbackSPU = SPU( 'readback' )
	for (name, value) in READBACK_OPTIONS:
		readbackSPU.Conf(name, value)
	node.AddSPU( readbackSPU )
	packSPU = SPU( 'pack' )
	node.AddSPU( packSPU )
	packSPU.AddServer( serverNode, 'tcpip' )

	appNodes.append( node )
	readbackSPUs.append( readbackSPU )
	cr.AddNode( node )


# Set mothership params
for (name, value) in MOTHERSHIP_OPTIONS:
	cr.Conf(name, value)

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
		id_Command         = 4006
		id_WindowSize      = 4007
		id_WindowResize    = 4008
		id_OK              = 4011
		id_CANCEL          = 4012
		id_HELP            = 4013

		# init misc member vars
		self.__Mothership = 0
		self.dirty = false

		# this sizer holds all the sortlast control widgets
		outerSizer = wxBoxSizer(wxVERTICAL)

		# Window width/height (in pixels)
		box = wxStaticBox(parent=self, id=-1, label="Initial Window Size",
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
		sizeChoices = []
		for i in CommonWindowSizes:
			sizeChoices.append( str("%d x %d" % (i[0], i[1])) )
		sizeChoices.append("Custom")
		self.sizeChoice = wxChoice(parent=self, id=id_WindowSize,
								   choices=sizeChoices)
		EVT_SPINCTRL(self.widthControl, id_Width, self.__OnSizeChange)
		EVT_SPINCTRL(self.heightControl, id_Height, self.__OnSizeChange)
		EVT_CHOICE(self.sizeChoice, id_WindowSize, self.__OnSizeChoice)
		windowSizeSizer.Add(widthLabel,
							flag=wxALIGN_CENTER_VERTICAL|wxALL, border=4)
		windowSizeSizer.Add(self.widthControl,
							flag=wxALIGN_CENTER_VERTICAL|wxALL, border=2)
		windowSizeSizer.Add(heightLabel,
							flag=wxALIGN_CENTER_VERTICAL|wxALL, border=4)
		windowSizeSizer.Add(self.heightControl,
							flag=wxALIGN_CENTER_VERTICAL|wxALL, border=2)
		windowSizeSizer.Add(self.sizeChoice,
							flag=wxALIGN_CENTER_VERTICAL|wxALL, border=2)
		outerSizer.Add(windowSizeSizer, flag=wxALL|wxGROW, border=4)

		# Resizable window option
		box = wxStaticBox(parent=self, id=-1, label="Window Resizing",
						  style=wxDOUBLE_BORDER)
		windowResizeSizer = wxStaticBoxSizer(box, wxHORIZONTAL)
		self.resizableButton = wxCheckBox(parent=self, id=id_WindowResize,
									 label="Resizable Window")
		EVT_CHECKBOX(self.resizableButton, id_WindowResize, self.__OnResizable)
		windowResizeSizer.Add(self.resizableButton)
		outerSizer.Add(windowResizeSizer, flag=wxALL|wxGROW, border=4)

		# Number of app nodes
		box = wxStaticBox(parent=self, id=-1, label="Application Nodes",
						  style=wxDOUBLE_BORDER)
		appSizer = wxStaticBoxSizer(box, wxHORIZONTAL)
		numberLabel = wxStaticText(parent=self, id=-1, label="Number:")
		self.numberControl = wxSpinCtrl(parent=self, id=id_NumApps,
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

		# horizontal separator (box with height=0)
#		separator = wxStaticBox(parent=self, id=-1,
#								label="", size=wxSize(10,0))
#		outerSizer.Add(separator, flag=wxGROW|wxALL, border=4)

		# Sizer for the OK, Cancel, Help buttons
		okCancelSizer = wxGridSizer(rows=1, cols=3, vgap=4, hgap=20)
		self.OkButton = wxButton(parent=self, id=id_OK, label="OK")
		okCancelSizer.Add(self.OkButton, option=0,
						  flag=wxALIGN_CENTER, border=0)
		self.CancelButton = wxButton(parent=self, id=id_CANCEL,
									 label="Cancel")
		okCancelSizer.Add(self.CancelButton, option=0,
						  flag=wxALIGN_CENTER, border=0)
		self.HelpButton = wxButton(parent=self, id=id_HELP,
									 label="Help")
		okCancelSizer.Add(self.HelpButton, option=0,
						  flag=wxALIGN_CENTER, border=0)
		EVT_BUTTON(self.OkButton, id_OK, self._onOK)
		EVT_BUTTON(self.CancelButton, id_CANCEL, self._onCancel)
		EVT_BUTTON(self.HelpButton, id_HELP, self._onHelp)
		outerSizer.Add(okCancelSizer, option=0, flag=wxALL|wxGROW, border=10)

		# Finish-up the dialog
		self.SetAutoLayout(true)
		self.SetSizer(outerSizer)

		minSize = outerSizer.GetMinSize()
		self.SetSizeHints(minW=minSize[0], minH=minSize[1])
		self.SetSize(minSize)

		# Hostname dialog
		self.hostsDialog = hostdialog.HostDialog(parent=NULL, id=-1,
						title="Chromium Hosts",
						message="Specify host names for the readback nodes")
		self.hostsDialog.Centre()

	# end of __init__()

	def __UpdateSizeWidgets(self):
		w = self.widthControl.GetValue()
		h = self.heightControl.GetValue()
		for i in range(0, len(CommonWindowSizes)):
			if w == CommonWindowSizes[i][0] and h == CommonWindowSizes[i][1]:
				self.sizeChoice.SetSelection(i)
				return
		# must be custom size
		self.sizeChoice.SetSelection(len(CommonWindowSizes))  # "Custom"
		
	def __UpdateVarsFromWidgets(self):
		"""Get current widget values and update the sortlast parameters."""
		renderSPU = FindRenderSPU(self.__Mothership)
		readbackSPU = FindReadbackSPU(self.__Mothership)
		appNode = FindClientNode(self.__Mothership)
		appNode.SetCount( self.numberControl.GetValue() )
		geom = readbackSPU.GetOption("window_geometry")
		geom[2] = self.widthControl.GetValue()
		geom[3] = self.heightControl.GetValue()
		readbackSPU.SetOption("window_geometry", geom)
		geom = renderSPU.GetOption("window_geometry")
		geom[2] = self.widthControl.GetValue()
		geom[3] = self.heightControl.GetValue()
		renderSPU.SetOption("window_geometry", geom)
		resizable = self.resizableButton.GetValue()
		renderSPU.SetOption("resizable", [resizable])
		readbackSPU.SetOption("resizable", [resizable])

	def __UpdateWidgetsFromVars(self):
		"""Set widget values to the tilesort parameters."""
		renderSPU = FindRenderSPU(self.__Mothership)
		readbackSPU = FindReadbackSPU(self.__Mothership)
		appNode = FindClientNode(self.__Mothership)
		self.numberControl.SetValue( appNode.GetCount() )
		geom = readbackSPU.GetOption("window_geometry")
		self.widthControl.SetValue(geom[2])
		self.heightControl.SetValue(geom[3])
		geom = renderSPU.GetOption("window_geometry")
		self.widthControl.SetValue(geom[2])
		self.heightControl.SetValue(geom[3])
		self.__UpdateSizeWidgets()
		resizable = renderSPU.GetOption("resizable")
		self.resizableButton.SetValue(resizable[0])


	# ----------------------------------------------------------------------
	# Event handling

	def __OnSizeChange(self, event):
		"""Called when window size changes with spin controls."""
		self.__UpdateSizeWidgets()
		self.dirty = true

	def __OnSizeChoice(self, event):
		"""Window size list choice callback."""
		i = self.sizeChoice.GetSelection()
		if i < len(CommonWindowSizes):
			w = CommonWindowSizes[i][0]
			h = CommonWindowSizes[i][1]
			self.widthControl.SetValue(w)
			self.heightControl.SetValue(h)
		else:
			self.__UpdateSizeWidgets()
		self.__UpdateVarsFromWidgets()
		self.dirty = true

	def __OnResizable(self, event):
		"""Called when window resizable checkbox changes."""
		# Update the SPU's resizable option to match checkbox state
		resizable = self.resizableButton.GetValue()
		renderSPU = FindRenderSPU(self.__Mothership)
		readbackSPU = FindReadbackSPU(self.__Mothership)
		renderSPU.SetOption("resizable", [resizable])
		readbackSPU.SetOption("resizable", [resizable])
		self.dirty = true

	def __OnNumAppsChange(self, event):
		"""Called when number of app nodes spin control changes."""
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
		# create the dialog
		dialog = spudialog.SPUDialog(parent=NULL, id=-1,
									 title="Readback SPU Options",
									 optionList = readbackSPU.GetOptions())
		# wait for OK or cancel
		if dialog.ShowModal() == wxID_OK:
			# save the new values/options
			for opt in readbackSPU.GetOptions():
				value = dialog.GetValue(opt.Name)
				readbackSPU.SetOption(opt.Name, value)
		return

	def __OnRenderOptions(self, event):
		"""Called when Render SPU Options button is pressed."""
		renderSPU = FindRenderSPU(self.__Mothership)
		# create the dialog
		dialog = spudialog.SPUDialog(parent=NULL, id=-1,
									 title="Render SPU Options",
									 optionList = renderSPU.GetOptions())
		# wait for OK or cancel
		if dialog.ShowModal() == wxID_OK:
			# save the new values/options
			for opt in renderSPU.GetOptions():
				value = dialog.GetValue(opt.Name)
				renderSPU.SetOption(opt.Name, value)
		return

	def _onOK(self, event):
		"""Called by OK button"""
		self.__UpdateVarsFromWidgets()
		self.EndModal(wxID_OK)

	def _onCancel(self, event):
		"""Called by Cancel button"""
		self.EndModal(wxID_CANCEL)

	def _onHelp(self, event):
		"""Called by Help button"""
		d = textdialog.TextDialog(parent=self, id = -1,
								  title="Sort-last Help")
		d.LoadPage("../../doc/sortlast_template.html")
		# Hmmm, I want to call d.Show() so this isn't modal/blocking but
		# that doesn't seem to work.
		d.ShowModal();
		d.Destroy()

	def SetMothership(self, mothership):
		"""Specify the mothership to modify.
		mothership is a Mothership object.
		"""
		self.__Mothership = mothership
		# update all the widgets to the template's values
		self.__UpdateWidgetsFromVars()



#----------------------------------------------------------------------


class SortlastTemplate(templatebase.TemplateBase):
	"""Template for creating/editing/reading/writing tilesort configs."""
	def __init__(self):
		# no-op for now
		pass
		
	def Name(self):
		return "Sort-last"

	def Create(self, parentWindow, mothership):
		"""Create the nodes/spus/etc for a sort-last config."""

		# Yes, client/server are transposed here
		appHosts = crutils.GetSiteDefault("cluster_hosts")
		if not appHosts:
			appHosts = ["localhost"]

		serverHosts = crutils.GetSiteDefault("frontend_hosts")
		if not serverHosts:
			serverHosts = ["localhost"]

		defaultNodes = len(appHosts)
		(defaultWidth, defaultHeight) = crutils.GetSiteDefault("screen_size")
		if defaultWidth < 128:
			defaultWidth = 128
		if defaultHeight < 128:
			defaultHeight = 128

		dialog = intdialog.IntDialog(parent=parentWindow, id=-1,
								 title="Sort-last Template",
								 labels=["Number of application nodes:",
										 "Initial Window Width:",
										 "Initial Window Height:"],
								 defaultValues=[defaultNodes,
												defaultWidth, defaultHeight],
								 minValue=1, maxValue=10000)
		dialog.Centre()
		if dialog.ShowModal() == wxID_CANCEL:
			dialog.Destroy()
			return 0
		numApps = dialog.GetValues()[0]
		width = dialog.GetValues()[1]
		height = dialog.GetValues()[2]

		mothership.Sortlast = SortlastParameters()

		mothership.DeselectAllNodes()

		# Create the server/render node
		xPos = 300
		yPos = 5
		serverNode = crtypes.NetworkNode(serverHosts, 1)
		serverNode.SetPosition(xPos, yPos)
		serverNode.Select()
		renderSPU = crutils.NewSPU("render")
		renderSPU.SetOption("window_geometry", [0, 0, width, height])
		serverNode.AddSPU(renderSPU)
		serverNode.SetOption( "only_swap_once", [1] )
		mothership.AddNode(serverNode)

		# Create the client/app nodes
		xPos = 5
		yPos = 5
		appNode = crtypes.ApplicationNode(appHosts, numApps)
		cluster_pattern = crutils.GetSiteDefault("cluster_pattern")
		if cluster_pattern:
			appNode.SetHostNamePattern(cluster_pattern);
		appNode.SetPosition(xPos, yPos)
		appNode.Select()
		readbackSPU = crutils.NewSPU("readback")
		readbackSPU.SetOption("title", ["Chromium Readback SPU"])
		readbackSPU.SetOption("window_geometry", [0, 0, width, height])
		readbackSPU.SetOption("extract_depth", [1])
		readbackSPU.Conf("extract_depth", 1)
		appNode.AddSPU(readbackSPU)
		packSPU = crutils.NewSPU("pack")
		appNode.AddSPU(packSPU)
		mothership.AddNode(appNode)
		packSPU.AddServer(serverNode)
		dialog.Destroy()
		return 1

	def Validate(self, mothership):
		"""Test if the mothership config is a sort-last config."""
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

	def Edit(self, parentWindow, mothership):
		"""Open editor window and edit the mothership config"""
		if not self.Validate(mothership):
			print "Editing - This is not a sort-last config!!!!"
			return 0

		d = SortlastDialog(parent=parentWindow)
		d.Centre()
		backupSortlastParams = mothership.Sortlast.Clone()
		d.SetMothership(mothership)

		if d.ShowModal() == wxID_CANCEL:
			# restore original values
			mothership.Sortlast = backupSortlastParams
		else:
			# update mothership with new values
			# already done in __UpdateVarsFromWidgets()
			pass
		return 1

	def Read(self, mothership, fileHandle):
		"""Read sort-last config from file"""
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

		# useful regex patterns
		integerPat = "[0-9]+"
		listPat = "\[.+\]"
		tuplePat = "\(.+\)"
		quotedStringPat = '\"(.*)\"'

		while true:
			l = fileHandle.readline()
			if not l:
				break
			# remove trailing newline character
			if l[-1:] == '\n':
				l = l[:-1]
			if re.match("^import", l):
				pass  # ignore
			elif re.match("^sys.path.append", l):
				pass  # ignore
			elif re.match("from mothership import", l):
				pass  # ignore
			elif re.match("^COMPOSITE_HOST = ", l):
				v = re.search(quotedStringPat + "$", l)
				host = l[v.start(1) : v.end(1)]
				serverNode.SetHosts( [ host ] )
			elif re.match("^NUM_APP_NODES = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				clientNode.SetCount( int(l[v.start() : v.end()]) )
			elif re.match("^APP_HOSTS = ", l):
				v = re.search(listPat + "$", l)
				hosts = eval(l[v.start() : v.end()])
				clientNode.SetHosts( hosts )
			elif re.match("^APP_PATTERN = ", l):
				v = re.search(tuplePat + "$", l)
				pattern = eval(l[v.start() : v.end()])
				clientNode.SetHostNamePattern(pattern)
			elif re.match("^READBACK_OPTIONS = \[", l):
				readbackSPU.GetOptions().Read(fileHandle)
			elif re.match("^RENDER_OPTIONS = \[", l):
				renderSPU.GetOptions().Read(fileHandle)
			elif re.match("^SERVER_OPTIONS = \[", l):
				serverNode.GetOptions().Read(fileHandle)
			elif re.match("^APP_OPTIONS = \[", l):
				substitutions = [ ("crbindir", "'%s'" % crconfig.crbindir) ]
				clientNode.GetOptions().Read(fileHandle, substitutions)
			elif re.match("^MOTHERSHIP_OPTIONS = \[", l):
				mothership.GetOptions().Read(fileHandle)
			elif re.match("^# end of options", l):
				# that's the end of the variables
				# save the rest of the file....
				break
			elif (l != "") and (not re.match("\s*#", l)):
				print "unrecognized line: %s" % l
		# endwhile

		mothership.LayoutNodes()
		return 1  # OK

	def Write(self, mothership, file):
		"""Write a sort-last config to the file handle."""
		if not self.Validate(mothership):
			print "Write: this isn't a sort-last config!"
			return 0

		sortlast = mothership.Sortlast
		clientNode = FindClientNode(mothership)
		serverNode = FindServerNode(mothership)

		file.write('TEMPLATE = "%s"\n' % self.Name())
		file.write(_ImportsSection)
		file.write('COMPOSITE_HOST = "%s"\n' % serverNode.GetHosts()[0])
		file.write('NUM_APP_NODES = %d\n' % clientNode.GetCount())
		file.write('APP_HOSTS = %s\n' % str(clientNode.GetHosts()))
		file.write('APP_PATTERN = %s\n' % str(clientNode.GetHostNamePattern()))

		# write render SPU options
		renderSPU = FindRenderSPU(mothership)
		renderSPU.GetOptions().Write(file, "RENDER_OPTIONS")

		# write readback SPU options
		readbackSPU = FindReadbackSPU(mothership)
		readbackSPU.GetOptions().Write(file, "READBACK_OPTIONS")

		# write server options
		serverNode.GetOptions().Write(file, "SERVER_OPTIONS")

		# write app node options
		substitutions = [ (crconfig.crbindir, 'crbindir') ]
		clientNode.GetOptions().Write(file, "APP_OPTIONS", substitutions)

		# write mothership options
		mothership.GetOptions().Write(file, "MOTHERSHIP_OPTIONS")

		file.write("# end of options, the rest is boilerplate (do not remove this line!)\n")
		file.write(_ConfigBody)
		return 1  # OK


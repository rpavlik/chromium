# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

""" tilesort_template.py
    Tilesort template module.
"""

# The bulk of this module is GUI code.  There are three public entrypoints
# to this code:
#   Create_Tilesort() - instantiate a tilesort template
#   Is_Tilesort() - test if the mothership config is a tilesort config
#   Edit_Tilesort() - specialized editor for tilesort configs



import string, cPickle, os.path, re
from wxPython.wx import *
import traceback, types
import intdialog, spudialog, hostdialog
import crutils, crtypes, configio



class TilesortParameters:
	"""C-style struct describing a tilesort configuration"""
	# This is where we set all the default tilesort parameters.
	def __init__(self, rows=1, cols=2):
		assert rows >= 1
		assert cols >= 1
		self.Columns = cols
		self.Rows = rows
		self.TileWidth = 1024
		self.TileHeight = 1024
		self.RightToLeft = 0
		self.BottomToTop = 0

	def Clone(self):
		"""Return a clone of this object."""
		# We're not using the copy.copy() function since it's flakey
		p = TilesortParameters()
		p.Columns = self.Columns
		p.Rows = self.Rows
		p.TileWidth = self.TileWidth
		p.TileHeight = self.TileHeight
		p.RightToLeft = self.RightToLeft
		p.BottomToTop = self.BottomToTop
		return p



# Predefined tile sizes shown in the wxChoice widget (feel free to change)
CommonTileSizes = [ [128, 128],
					[256, 256],
					[512, 512],
					[1024, 1024],
					[1280, 1024],
					[1600, 1200] ]

BackgroundColor = wxColor(70, 170, 130)


#----------------------------------------------------------------------------

# This is the guts of the tilesort configuration script.
# It's simply appended to the file after we write all the configuration options
__ConfigBody = """
import string, sys
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

# Determine if tiles are on one server or many
if (len(SERVER_HOSTS) >= 2) and (SERVER_HOSTS[0] != SERVER_HOSTS[1]):
	singleServer = 0
else:
	singleServer = 1

localHostname = os.uname()[1]

cr = CR()
cr.MTU( GLOBAL_MTU )


tilesortSPUs = []
clientNodes = []

for i in range(NUM_CLIENTS):
	tilesortspu = SPU('tilesort')
	tilesortspu.Conf('broadcast', TILESORT_broadcast)
	tilesortspu.Conf('optimize_bucket', TILESORT_optimize_bucket)
	tilesortspu.Conf('sync_on_swap', TILESORT_sync_on_swap)
	tilesortspu.Conf('sync_on_finish', TILESORT_sync_on_finish)
	tilesortspu.Conf('draw_bbox', TILESORT_draw_bbox)
	tilesortspu.Conf('bbox_line_width', TILESORT_bbox_line_width)
	#tilesortspu.Conf('fake_window_dims', fixme)
	tilesortspu.Conf('scale_to_mural_size', TILESORT_scale_to_mural_size)
	tilesortSPUs.append(tilesortspu)

	clientnode = CRApplicationNode()
	clientnode.AddSPU(tilesortspu)

	clientnode.StartDir( crbindir )
	clientnode.SetApplication( os.path.join(crbindir, program) )
	clientnode.StartDir( GLOBAL_default_dir )
	if GLOBAL_auto_start:
		clientnode.AutoStart( ["/bin/sh", "-c",
				"LD_LIBRARY_PATH=%s /usr/local/bin/crappfaker" % crlibdir] )

	clientNodes.append(clientnode)


for row in range(TILE_ROWS):
	for col in range(TILE_COLS):

		# layout directions
		if RIGHT_TO_LEFT:
			j = TILE_COLS - col - 1
		else:
			j = col
		if BOTTOM_TO_TOP:
			i = TILE_ROWS - row - 1
		else:
			i = row

		# compute index for this tile
		index = i * TILE_COLS + j

		renderspu = SPU('render')
		renderspu.Conf('try_direct', RENDER_try_direct)
		renderspu.Conf('force_direct', RENDER_force_direct)
		renderspu.Conf('fullscreen', RENDER_fullscreen)
		renderspu.Conf('title', RENDER_title)
		renderspu.Conf('system_gl_path', RENDER_system_gl_path)

		if singleServer:
			renderspu.Conf('window_geometry',
						   int(1.1 * j * TILE_WIDTH),
						   int(1.1 * i * TILE_HEIGHT),
						   TILE_WIDTH, TILE_HEIGHT)
			host = SERVER_HOSTS[0]
		else:
			renderspu.Conf('window_geometry', 0, 0, TILE_WIDTH, TILE_HEIGHT)
			host = SERVER_HOSTS[index]
		servernode = CRNetworkNode(host)

		servernode.AddTile(col * TILE_WIDTH,
						   (TILE_ROWS - row - 1) * TILE_HEIGHT,
						   TILE_WIDTH, TILE_HEIGHT)

		servernode.AddSPU(renderspu)
		servernode.Conf('optimize_bucket', SERVER_optimize_bucket)

		cr.AddNode(servernode)
		for i in range(NUM_CLIENTS):
			tilesortSPUs[i].AddServer(servernode, protocol='tcpip', port = 7000 + index)

		if GLOBAL_auto_start:
			servernode.AutoStart( ["/usr/bin/rsh", host,
									"/bin/sh -c 'DISPLAY=:0.0  CRMOTHERSHIP=%s  LD_LIBRARY_PATH=%s  crserver'" % (localHostname, crlibdir) ] )


for i in range(NUM_CLIENTS):
	cr.AddNode(clientNodes[i])
cr.SetParam('minimum_window_size', GLOBAL_minimum_window_size)
cr.SetParam('match_window_title', GLOBAL_match_window_title)
cr.SetParam('show_cursor', GLOBAL_show_cursor)
cr.Go()

"""

#----------------------------------------------------------------------------

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

def FindTilesortSPU(mothership):
	"""Search the mothership for the tilesort SPU."""
	appNode = FindClientNode(mothership)
	tilesortSPU = appNode.LastSPU()
	assert tilesortSPU.Name() == "tilesort"
	return tilesortSPU

def FindRenderSPU(mothership):
	"""Search the mothership for the render SPU."""
	serverNode = FindServerNode(mothership)
	renderSPU = serverNode.LastSPU()
	assert renderSPU.Name() == "render"
	return renderSPU

#----------------------------------------------------------------------------

class TilesortDialog(wxDialog):
	"""Tilesort configuration editor."""

	def __init__(self, parent=NULL, id=-1):
		""" Construct a TilesortFrame."""
		wxDialog.__init__(self, parent, id, title="Tilesort Configuration",
						 style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)

		# Widget IDs
		id_MuralWidth  = 3000
		id_MuralHeight = 3001
		id_TileChoice  = 3002
		id_TileWidth   = 3003
		id_TileHeight  = 3004
		id_hLayout     = 3005
		id_vLayout     = 3006
		id_HostText    = 3007
		id_HostIndex   = 3008
		id_TilesortOptions = 3009
		id_Hostnames   = 3010
		id_OK          = 3011
		id_CANCEL      = 3012

		# init misc member vars
		self.__Mothership = 0
		self.dirty = false

		# this sizer holds all the tilesort control widgets
		toolSizer = wxBoxSizer(wxVERTICAL)

		# Mural width/height (in tiles)
		box = wxStaticBox(parent=self, id=-1, label="Mural Size",
						  style=wxDOUBLE_BORDER)
		muralSizer = wxStaticBoxSizer(box, wxVERTICAL)
		flexSizer = wxFlexGridSizer(rows=2, cols=2, hgap=4, vgap=4)
		columnsLabel = wxStaticText(parent=self, id=-1,
									label="Columns:")
		self.widthControl = wxSpinCtrl(parent=self, id=id_MuralWidth,
									   value="1", min=1, max=16,
									   size=wxSize(50,25))
		rowsLabel = wxStaticText(parent=self, id=-1, label="Rows:")
		self.heightControl = wxSpinCtrl(parent=self,
										id=id_MuralHeight,
										value="1", min=1, max=16,
										size=wxSize(50,25))
		EVT_SPINCTRL(self.widthControl, id_MuralWidth, self.__OnSizeChange)
		EVT_SPINCTRL(self.heightControl, id_MuralHeight, self.__OnSizeChange)
		flexSizer.Add(columnsLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.widthControl)
		flexSizer.Add(rowsLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.heightControl)
		muralSizer.Add(flexSizer)
		toolSizer.Add(muralSizer, flag=wxEXPAND)

		# Tile size (in pixels)
		box = wxStaticBox(parent=self, id=-1, label="Tile Size",
						  style=wxDOUBLE_BORDER)
		tileSizer = wxStaticBoxSizer(box, wxVERTICAL)
		flexSizer = wxFlexGridSizer(rows=2, cols=2, hgap=4, vgap=4)
		tileChoices = []
		for i in CommonTileSizes:
			tileChoices.append( str("%d x %d" % (i[0], i[1])) )
		tileChoices.append("Custom")
		self.tileChoice = wxChoice(parent=self, id=id_TileChoice,
								   choices=tileChoices)
		flexSizer = wxFlexGridSizer(rows=2, cols=2, hgap=4, vgap=4)
		self.tileWidthLabel = wxStaticText(parent=self, id=-1,
										   label="Width:")
		self.tileWidthControl = wxSpinCtrl(parent=self,
										   id=id_TileWidth,
										   value="512", min=128, max=2048,
										   size=wxSize(80,25))
		self.tileHeightLabel = wxStaticText(parent=self, id=-1,
											label="Height:")
		self.tileHeightControl = wxSpinCtrl(parent=self,
											id=id_TileHeight,
											value="512", min=128, max=2048,
											size=wxSize(80,25))
		EVT_SPINCTRL(self.tileWidthControl, id_TileWidth, self.__OnSizeChange)
		EVT_SPINCTRL(self.tileHeightControl, id_TileHeight, self.__OnSizeChange)
		EVT_CHOICE(self.tileChoice, id_TileChoice, self.__OnTileChoice)
		flexSizer.Add(self.tileWidthLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.tileWidthControl)
		flexSizer.Add(self.tileHeightLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.tileHeightControl)
		tileSizer.Add(self.tileChoice, flag=wxALIGN_CENTER)
		tileSizer.Add(flexSizer)
		toolSizer.Add(tileSizer, flag=wxEXPAND)

		# Total mural size (in pixels)
		box = wxStaticBox(parent=self, id=-1, label="Total Size",
						  style=wxDOUBLE_BORDER)
		totalSizer = wxStaticBoxSizer(box, wxVERTICAL)
		self.totalSizeLabel = wxStaticText(parent=self, id=-1,
										   label="??")
		totalSizer.Add(self.totalSizeLabel, flag=wxEXPAND)
		toolSizer.Add(totalSizer, flag=wxEXPAND)

		hChoices = [ 'Left to right', 'Right to left' ]
		self.hLayoutRadio = wxRadioBox(parent=self, id=id_hLayout,
									   label="Horizontal Layout",
									   choices=hChoices,
									   majorDimension=1,
									   style=wxRA_SPECIFY_COLS )
		toolSizer.Add(self.hLayoutRadio, flag=wxEXPAND)
		vChoices = [ 'Top to bottom', 'Bottom to top' ]
		self.vLayoutRadio = wxRadioBox(parent=self, id=id_vLayout,
									   label="Vertical Layout",
									   choices=vChoices,
									   majorDimension=1,
									   style=wxRA_SPECIFY_COLS )
		toolSizer.Add(self.vLayoutRadio, flag=wxEXPAND)
		EVT_RADIOBOX(self.hLayoutRadio, id_hLayout, self.__OnLayoutChange)
		EVT_RADIOBOX(self.vLayoutRadio, id_vLayout, self.__OnLayoutChange)

		# Hostname dialog
		self.hostsDialog = hostdialog.HostDialog(parent=NULL, id=-1,
						title="Chromium Hosts",
						message="Specify host names for the tile servers")

		# Hostname button
		self.hostsButton = wxButton(parent=self, id=id_Hostnames,
									label=" Tile Host Names... ")
		toolSizer.Add(self.hostsButton, flag=wxALL|wxALIGN_CENTRE_HORIZONTAL,
					  border=2)
		EVT_BUTTON(self.hostsButton, id_Hostnames, self.__OnHostnames)

		# Tilesort SPU option button
		self.tilesortButton = wxButton(parent=self, id=id_TilesortOptions,
									   label=" Tilesort SPU Options... ")
		toolSizer.Add(self.tilesortButton, flag=wxALL, border=2)
		EVT_BUTTON(self.tilesortButton, id_TilesortOptions,
				   self.__OnTilesortOptions)

		# Setup the drawing area
		self.drawArea = wxPanel(self, id=-1, style=wxSUNKEN_BORDER)
		self.drawArea.SetBackgroundColour(BackgroundColor)
		EVT_PAINT(self.drawArea, self.__OnPaintEvent)

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

		# The toolAndDrawSizer contains the toolpanel and drawing area
		toolAndDrawSizer = wxBoxSizer(wxHORIZONTAL)
		toolAndDrawSizer.Add(toolSizer, option=0,
 					 flag=wxALL|wxALIGN_TOP, border=4)
		toolAndDrawSizer.Add(self.drawArea, 1, wxEXPAND)

		# horizontal separator (box with height=0)
		separator = wxStaticBox(parent=self, id=-1,
								label="", size=wxSize(100,0))

		# The topsizer contains the toolAndDrawSizer and the okCancelSizer
		topSizer = wxBoxSizer(wxVERTICAL)
		topSizer.Add(toolAndDrawSizer, flag=wxGROW, border=0)
		topSizer.Add(separator, flag=wxGROW, border=0)
		topSizer.Add(okCancelSizer, option=0, flag=wxGROW|wxTOP, border=10)

		self.SetAutoLayout(true)
		self.SetSizer(topSizer)

		minSize = topSizer.GetMinSize()
		minSize[0] = 600
		minSize[1] += 10
		self.SetSizeHints(minW=400, minH=minSize[1])
		self.SetSize(minSize)

		self.__RecomputeTotalSize()
	# end of __init__()

	def __RecomputeTotalSize(self):
		"""Called whenever the mural width/height or tile width/height changes.
		Recompute the total mural size in pixels and update the widgets."""
		tileW = self.tileWidthControl.GetValue()
		tileH = self.tileHeightControl.GetValue()
		totalW = self.widthControl.GetValue() * tileW
		totalH = self.heightControl.GetValue() * tileH
		self.totalSizeLabel.SetLabel(str("%d x %d" % (totalW, totalH)))
		custom = 1
		for i in range(0, len(CommonTileSizes)):
			if (tileW == CommonTileSizes[i][0] and
				tileH == CommonTileSizes[i][1]):
				self.tileChoice.SetSelection(i)
				return
		# must be custom size
		self.tileChoice.SetSelection(len(CommonTileSizes))  # "Custom"

	def __UpdateVarsFromWidgets(self):
		"""Get current widget values and update the tilesort parameters."""
		tilesort = self.__Mothership.Tilesort
		tilesort.Columns = self.widthControl.GetValue()
		tilesort.Rows = self.heightControl.GetValue()
		tilesort.TileWidth = self.tileWidthControl.GetValue()
		tilesort.TileHeight = self.tileHeightControl.GetValue()
		tilesort.RightToLeft = self.hLayoutRadio.GetSelection()
		tilesort.BottomToTop = self.vLayoutRadio.GetSelection()

	def __UpdateWidgetsFromVars(self):
		"""Set widget values to the tilesort parameters."""
		tilesort = self.__Mothership.Tilesort
		self.widthControl.SetValue(tilesort.Columns)
		self.heightControl.SetValue(tilesort.Rows)
		self.tileWidthControl.SetValue(tilesort.TileWidth)
		self.tileHeightControl.SetValue(tilesort.TileHeight)
		self.hLayoutRadio.SetSelection(tilesort.RightToLeft)
		self.vLayoutRadio.SetSelection(tilesort.BottomToTop)

	# ----------------------------------------------------------------------
	# Event handling

	def __OnSizeChange(self, event):
		"""Called when tile size changes with spin controls."""
		self.__UpdateVarsFromWidgets()
		self.__RecomputeTotalSize()
		self.drawArea.Refresh()
		self.dirty = true

	def __OnLayoutChange(self, event):
		"""Called when left/right top/bottom layout changes."""
		self.__UpdateVarsFromWidgets()
		self.__RecomputeTotalSize()
		self.drawArea.Refresh()
		self.dirty = true

	def __OnTileChoice(self, event):
		"""Called when tile size changes with combo-box control."""
		i = self.tileChoice.GetSelection()
		if i < len(CommonTileSizes):
			w = CommonTileSizes[i][0]
			h = CommonTileSizes[i][1]
			self.tileWidthControl.SetValue(w)
			self.tileHeightControl.SetValue(h)
		self.__UpdateVarsFromWidgets()
		self.__RecomputeTotalSize()
		self.drawArea.Refresh()
		self.dirty = true

	def __OnHostNameChange(self, event):
		"""Called when the host name pattern changes."""
		self.__UpdateVarsFromWidgets()
		self.drawArea.Refresh()
		self.dirty = true

	def __OnHostStartChange(self, event):
		"""Called when the first host index changes."""
		self.__UpdateVarsFromWidgets()
		self.drawArea.Refresh()
		self.dirty = true

	def __OnHostnames(self, event):
		"""Called when the hostnames button is pressed."""
		tilesort = self.__Mothership.Tilesort
		serverNode = FindServerNode(self.__Mothership)
		self.hostsDialog.SetHostPattern(serverNode.GetHostNamePattern())
		self.hostsDialog.SetCount(tilesort.Rows * tilesort.Columns)
		self.hostsDialog.SetHosts(serverNode.GetHosts())
		if self.hostsDialog.ShowModal() == wxID_OK:
			serverNode.SetHostNamePattern(self.hostsDialog.GetHostPattern())
			serverNode.SetHosts(self.hostsDialog.GetHosts())
		self.drawArea.Refresh()

	def __OnTilesortOptions(self, event):
		"""Called when Tilesort Options button is pressed."""
		tilesortSPU = FindTilesortSPU(self.__Mothership)
		(params, opts) = crutils.GetSPUOptions("tilesort")
		# create the dialog
		dialog = spudialog.SPUDialog(parent=NULL, id=-1,
									 title="Tilesort SPU Options",
									 options = opts)
		# set the dialog widget values
		dialog.SetValues(tilesortSPU.GetOptions())
		# wait for OK or cancel
		if dialog.ShowModal() == wxID_OK:
			# save the new values/options
			tilesortSPU.SetOptions(dialog.GetValues())
		else:
			# user cancelled, do nothing, new values are ignored
			pass

	def _onOK(self, event):
		"""Called by OK button"""
		self.EndModal(wxID_OK)

	def _onCancel(self, event):
		"""Called by Cancel button"""
		self.EndModal(wxID_CANCEL)

	def __OnPaintEvent(self, event):
		""" Respond to a request to redraw the contents of our drawing panel.
		"""
		dc = wxPaintDC(self.drawArea)
		#self.drawArea.PrepareDC(dc)  # only for scrolled windows
		dc.BeginDrawing()
		dc.SetPen(wxBLACK_PEN);
		dc.SetBrush(wxLIGHT_GREY_BRUSH);

		# border around the window and space between the tiles
		border = 10
		space = 2
		
		# Get current settings
		size = self.drawArea.GetSize()
		cols = self.widthControl.GetValue()
		rows = self.heightControl.GetValue()
		tileWidth = self.tileWidthControl.GetValue()
		tileHeight = self.tileHeightControl.GetValue()
		lToR = self.hLayoutRadio.GetSelection()
		tToB = self.vLayoutRadio.GetSelection()

		# how many pixels we'd like to draw
		desiredWidth = cols * tileWidth
		desiredHeight = rows * tileHeight

		# how much space we have in the window
		availWidth = size.width - 2 * border - (cols + 1) * space
		availHeight = size.height - 2 * border - (rows + 1) * space
		
		if desiredWidth > availWidth or desiredHeight > availHeight:
			# we need to draw smaller tiles to make them all fit
			xScale = float(availWidth) / float(desiredWidth)
			yScale = float(availHeight) / float (desiredHeight)
			if xScale > yScale:
				scale = yScale
			else:
				scale = xScale
			if scale > 0.1:
				scale = 0.1
		else:
			# all the tiles will fit at 1/10 scale factor
			scale = 0.1

		# draw the tiles as boxes
		w = tileWidth * scale
		h = tileHeight * scale
		serverNode = FindServerNode(self.__Mothership)
		hosts = serverNode.GetHosts()
		for i in range(rows):
			for j in range(cols):
				x = j * (w + space) + border
				y = i * (h + space) + border
				dc.DrawRectangle(x, y, w, h)
				if (tToB == 0):
					ii = i
				else:
					ii = rows - i - 1
				if (lToR == 0):
					jj = j
				else:
					jj = cols - j - 1
				k = ii * cols + jj
				if k < len(hosts):
					s = hosts[k]
				else:
					s = hosts[-1]
				dc.DrawText(s, x+3, y+3)
		dc.EndDrawing()

	def SetMothership(self, mothership):
		"""Specify the mothership to modify.
		mothership is a Mothership object.
		"""
		self.__Mothership = mothership
		# update all the widgets to the template's values
		self.__UpdateWidgetsFromVars()
		self.__RecomputeTotalSize()


#----------------------------------------------------------------------
# Global entrypoints called from the templates.py module, everthing else
# above here is private.

def Create_Tilesort(parentWindow, mothership):
	"""Create a tilesort configuration"""

	defaultMuralSize = crutils.GetSiteDefault("mural_size")
	if defaultMuralSize:
		dialogDefaults = [ 1, defaultMuralSize[0], defaultMuralSize[1] ]
	else:
		dialogDefaults = [1, 2, 1]

	dialog = intdialog.IntDialog(NULL, id=-1, title="Tilesort Template",
								 labels=["Number of application nodes:",
										 "Mural Columns:",
										 "Mural Rows:"],
								 defaultValues=dialogDefaults, maxValue=10000)
	if dialog.ShowModal() == wxID_CANCEL:
		dialog.Destroy()
		return 0

	# Init tilesort parameters
	values = dialog.GetValues()
	numClients = values[0]
	cols = values[1]
	rows = values[2]
	mothership.Tilesort = TilesortParameters(rows, cols)

	defaultScreenSize = crutils.GetSiteDefault("screen_size")
	if defaultScreenSize:
		mothership.Tilesort.TileWidth = defaultScreenSize[0]
		mothership.Tilesort.TileHeight = defaultScreenSize[1]

	# build the graph
	numServers = rows * cols
	mothership.DeselectAllNodes()
	# Create the <numClients> app nodes
	appNode = crutils.NewApplicationNode(numClients)
	appNode.SetPosition(50, 50)
	appNode.Select()
	tilesortSPU = crutils.NewSPU("tilesort")
	appNode.AddSPU(tilesortSPU)
	mothership.AddNode(appNode)
	# Create the <numServers> server nodes
	serverNode = crutils.NewNetworkNode(numServers)
	serverNode.SetPosition(350, 50)
	serverNode.Select()
	renderSPU = crutils.NewSPU("render")
	serverNode.AddSPU(renderSPU)
	mothership.AddNode(serverNode)
	tilesortSPU.AddServer(serverNode)
	# done with the dialog
	dialog.Destroy()
	return 1


def Is_Tilesort(mothership):
	"""Test if the mothership describes a tilesort configuration.
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
	# Next, check for correct SPU types
	if nodes[0].IsAppNode():
		tilesortSPU = nodes[0].LastSPU()
		serverNode = nodes[1]
		renderSPU = serverNode.LastSPU()
	else:
		tilesortSPU = nodes[1].LastSPU()
		serverNode = nodes[0]
		renderSPU = serverNode.LastSPU()
	if tilesortSPU.Name() != "tilesort":
		print "no tilesort SPU"
		return 0
	if renderSPU.Name() != "render":
		print "no render SPU"
		return 0
	# Next, check that the app's servers are correct
	servers = tilesortSPU.GetServers()
	if len(servers) != 1 or servers[0] != serverNode:
		print "no client/server connection"
		return 0
	# OK, this is a tilesort config!
	return 1


def Edit_Tilesort(parentWindow, mothership):
	"""Edit parameters for a Tilesort template"""
	# XXX we only need to create one instance of the TilesortFrame() and
	# reuse it in the future.
	t = Is_Tilesort(mothership)
	if t:
		# find the server/render nodes
		nodes = mothership.Nodes()
		if nodes[0].IsAppNode():
			clientNode = nodes[0]
			serverNode = nodes[1]
		else:
			clientNode = nodes[1]
			serverNode = nodes[0]
	else:
		print "This is not a tilesort configuration!"
		return

	d = TilesortDialog()
	d.Centre()
	backupTilesortParams = mothership.Tilesort.Clone()
	d.SetMothership(mothership)

	if d.ShowModal() == wxID_CANCEL:
		# restore original values
		mothership.Tilesort = backupTilesortParams
	else:
		# update mothership with new values
		tiles = mothership.Tilesort.Rows * mothership.Tilesort.Columns
		serverNode.SetCount(tiles)


def Read_Tilesort(mothership, fileHandle):
	"""Read a tilesort config from the given file handle."""

	mothership.Tilesort = TilesortParameters()

	serverNode = crtypes.NetworkNode()
	renderSPU = crutils.NewSPU("render")
	serverNode.AddSPU(renderSPU)

	clientNode = crtypes.ApplicationNode()
	tilesortSPU = crutils.NewSPU("tilesort")
	clientNode.AddSPU(tilesortSPU)
	tilesortSPU.AddServer(serverNode)

	mothership.AddNode(clientNode)
	mothership.AddNode(serverNode)

	while true:
		l = fileHandle.readline()
		if not l:
			break
		# remove trailing newline character
		if l[-1:] == '\n':
			l = l[:-1]
		if re.match("^TILE_ROWS = [0-9]+$", l):
			v = re.search("[0-9]+", l)
			mothership.Tilesort.Rows = int(l[v.start() : v.end()])
		elif re.match("^TILE_COLS = [0-9]+$", l):
			v = re.search("[0-9]+", l)
			mothership.Tilesort.Columns = int(l[v.start() : v.end()])
		elif re.match("^TILE_WIDTH = [0-9]+$", l):
			v = re.search("[0-9]+", l)
			mothership.Tilesort.TileWidth = int(l[v.start() : v.end()])
		elif re.match("^TILE_HEIGHT = [0-9]+$", l):
			v = re.search("[0-9]+", l)
			mothership.Tilesort.TileHeight = int(l[v.start() : v.end()])
		elif re.match("^BOTTOM_TO_TOP = [01]$", l):
			v = re.search("[01]", l)
			mothership.Tilesort.BottomToTop = int(l[v.start() : v.end()])
		elif re.match("^RIGHT_TO_LEFT = [01]$", l):
			v = re.search("[01]", l)
			mothership.Tilesort.RightToLeft = int(l[v.start() : v.end()])
		elif re.match("^SERVER_HOSTS = ", l):
			v = re.search("\[.+\]$", l)
			hosts = eval(l[v.start() : v.end()])
			serverNode.SetHosts(hosts)
		elif re.match("^SERVER_PATTERN = ", l):
			v = re.search("\(.+\)$", l)
			pattern = eval(l[v.start() : v.end()])
			serverNode.SetHostNamePattern(pattern)
		elif re.match("^NUM_CLIENTS = [0-9]+$", l):
			v = re.search("[0-9]+", l)
			numClients = int(l[v.start() : v.end()])
		elif re.match("^TILESORT_", l):
			# A tilesort SPU option
			(name, values) = configio.ParseOption(l, "TILESORT")
			tilesortSPU.SetOption(name, values)
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

	clientNode.SetCount(numClients)
	serverNode.SetCount(mothership.Tilesort.Rows * mothership.Tilesort.Columns)
	mothership.LayoutNodes()
	return 1


def Write_Tilesort(mothership, file):
	"""Write a tilesort config to the given file handle."""
	assert Is_Tilesort(mothership)
	assert mothership.GetTemplateType() == "Tilesort"

	print "Writing tilesort config"
	tilesort = mothership.Tilesort
	clientNode = FindClientNode(mothership)
	serverNode = FindServerNode(mothership)

	file.write('TEMPLATE = "Tilesort"\n')
	file.write("TILE_ROWS = %d\n" % tilesort.Rows)
	file.write("TILE_COLS = %d\n" % tilesort.Columns)
	file.write("TILE_WIDTH = %d\n" % tilesort.TileWidth)
	file.write("TILE_HEIGHT = %d\n" % tilesort.TileHeight)
	file.write("RIGHT_TO_LEFT = %d\n" % tilesort.RightToLeft)
	file.write("BOTTOM_TO_TOP = %d\n" % tilesort.BottomToTop)
	file.write("SERVER_HOSTS = %s\n" % str(serverNode.GetHosts()))
	file.write('SERVER_PATTERN = %s\n' % str(serverNode.GetHostNamePattern()))
	file.write("NUM_CLIENTS = %d\n" % clientNode.GetCount())

	# write tilesort SPU options
	tilesortSPU = FindTilesortSPU(mothership)
	configio.WriteSPUOptions(tilesortSPU, "TILESORT", file)

	# write render SPU options
	renderSPU = FindRenderSPU(mothership)
	configio.WriteSPUOptions(renderSPU, "RENDER", file)

	# write server and global options
	configio.WriteServerOptions(mothership, file)
	configio.WriteGlobalOptions(mothership, file)

	file.write("# end of options, the rest is boilerplate\n")
	file.write(__ConfigBody)

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

""" lightning2_template.py
    Template for setting up Lightning-2 (tile-reassembly) configurations.
"""


# Known issues:
# 1. Tiles must all be same size
# 2. Tiles cannot overlap
# 3. No support for the Lightning-2 border pixel / reassembly codes



import string, cPickle, os.path, re, sys
from wxPython.wx import *
import crtypes, crutils, intdialog, hostdialog, configio
sys.path.append( "../server" )
import tilelayout
import templatebase
import textdialog


class LightningParameters:
	"""This class describes the parameters of a Lightning-2 configuration.
	When we begin editing a Lightning-2 config we init these values from
	the mothership (number of servers, etc) and/or the initial create-
	lightning-2 dialog.
	When we finish editing, we update the mothership config."""
	def __init__(self, rows=1, cols=2):
		assert rows >= 1
		assert cols >= 1
		self.NumServers = 4
		self.ServerHosts = ["localhost"]
		self.ServerPattern = ("localhost", 1)
		self.Columns = cols
		self.Rows = rows
		self.TileWidth = 256
		self.TileHeight = 256
		self.ScreenWidth = 1280  # server screen
		self.ScreenHeight = 1024
		self.Layout = 0
		self.DynamicSize = 0   # 0, 1, or 2
		self.Reassembly = 0  # 0, 1, o

	def Clone(self):
		"""Return a clone of this object."""
		# We're not using the copy.copy() function since it's flakey
		p = LightningParameters()
		p.NumServers = self.NumServers
		p.ServerHosts = self.ServerHosts[:]
		p.ServerPattern = self.ServerPattern
		p.Columns = self.Columns
		p.Rows = self.Rows
		p.TileWidth = self.TileWidth
		p.TileHeight = self.TileHeight
		p.ScreenWidth = self.ScreenWidth
		p.ScreenHeight = self.ScreenHeight
		p.Layout = self.Layout
		p.DynamicSize = self.DynamicSize
		p.Reassembly = self.Reassembly
		return p

	def UpdateFromMothership(self, mothership):
		"""Update template values by examining the mothership/graph."""
		serverNode = FindServerNode(mothership)
		clientNode = FindClientNode(mothership)
		self.NumServers = serverNode.GetCount()
		self.ServerHosts = serverNode.GetHosts()[:] # [:] makes a copy
		self.ServerPattern = serverNode.GetHostNamePattern()
		#self.Columns = ??
		#self.Rows = ??
		#self.TileWidth = ??
		#self.TileHeight = ??
		#self.Layout = ??
		#self.DynamicSize = ??
		#self.Reassembly = ??
		
	def UpdateMothership(self, mothership):
		"""Update the mothership/graph from the template parameters."""
		clientNode = FindClientNode(mothership)
		serverNode = FindServerNode(mothership)
		serverNode.SetCount(self.NumServers)
		serverNode.SetHostNamePattern(self.ServerPattern)
		serverNode.SetHosts(self.ServerHosts)
		renderSPU = FindReassemblySPU(mothership)
		
		for i in range(serverNode.GetCount()):
			serverNode.SetTiles(self.MuralTiles[i], i)
		if self.DynamicSize:
			mothership.Conf('track_window_size', 1)
			renderSPU.Conf('resizable', 1)
		else:
			mothership.Conf('track_window_size', 0)
			renderSPU.Conf('resizable', 0)
		if self.Reassembly == 2:
			renderSPU.Conf('render_to_app_window', 1)
		else:
			renderSPU.Conf('render_to_app_window', 0)


	def __AllocTiles(self, numServers, tiles):
		"""Allocate space for the tiles on the servers.  This is where
		we'll determine if there's enough space in the server's rendering
		window to accomodate all the tiles.
		Input: tiles is a list of tuples (server, x, y, w, h).
		Result: self.MuralTiles and self.ServerTiles will be filled in.
		Return 1 for success, 0 if we run out of room on the server."""
		# XXX we're assuming all tiles are the same height here!

		# initialize tile lists
		self.MuralTiles = []   # array [server] of array of (x, y, w, h)
		nextTile = []          # array [server] of (x, y)
		for i in range(numServers):
			self.MuralTiles.append( [] )
			nextTile.append( (0, 0) )

		self.ServerTiles = tiles[:] # copy the list

		for (server, x, y, w, h) in tiles:
			assert server >= 0
			assert server < numServers
			assert numServers == len(self.MuralTiles)
			assert len(self.MuralTiles) == len(nextTile)
			# Check if tile of size (w, h) will fit on this server
			# We allocate tiles in raster order, as in the crserver.
			# (nextX, nextY) = position on server's screen
			(nextX, nextY) = nextTile[server]
			if nextY + h > self.ScreenHeight:
				# ran out of room on this server!!!
				return 0
			elif nextX + w > self.ScreenWidth:
				# go to next row
				nextX = 0
				nextY += h
				if nextY + h > self.ScreenHeight:
					# ran out of room on this server!!!
					return 0
			# It'll fit, save it
			muralTile = (x, y, w, h)  # omit server
			self.MuralTiles[server].append( muralTile )
			# Update NextTile position for this server
			nextX += w
			nextTile[server] = (nextX, nextY)
		return 1


	def PrintTiles(self):
		# for debug only
		for i in range(len(self.MuralTiles)):
			print "server %d" % i
			for tile in self.MuralTiles[i]:
				print "  (%d, %d, %d, %d)" % tile

	def LayoutTiles(self):
		"""Compute locations and hosts for the tiles."""

		# begin layout
		if self.Layout == 0:
			# Simple raster order layout
			muralWidth = self.TileWidth * self.Columns
			muralHeight = self.TileHeight * self.Rows
			tiles = tilelayout.LayoutRaster(muralWidth, muralHeight,
											self.NumServers,
											self.TileWidth, self.TileHeight,
											self.Rows, self.Columns)
		elif self.Layout == 1:
			# Slightly different raster order layout
			muralWidth = self.TileWidth * self.Columns
			muralHeight = self.TileHeight * self.Rows
			tiles = tilelayout.LayoutZigZag(muralWidth, muralHeight,
											self.NumServers,
											self.TileWidth, self.TileHeight,
											self.Rows, self.Columns)
		else:
			# Spiral outward from the center (this is a little tricky)
			assert self.Layout == 2

			muralWidth = self.TileWidth * self.Columns
			muralHeight = self.TileHeight * self.Rows
			tiles = tilelayout.LayoutSpiral(muralWidth, muralHeight,
											self.NumServers,
											self.TileWidth, self.TileHeight,
											self.Rows, self.Columns)
		# Allocate tiles on the server
		self.__AllocTiles(self.NumServers, tiles)
		return


# Predefined tile sizes shown in the wxChoice widget (feel free to change)
CommonTileSizes = [ [32, 32],
					[64, 64],
					[128, 128],
					[256, 256],
					[512, 512] ]

BackgroundColor = wxColor(70, 170, 130)

ServerColors = [
	wxColor(255, 50, 50),   # red
	wxColor(50, 255, 50),   # green
	wxColor(150, 150, 255), # blue
	wxColor(0, 200, 200),   # cyan
	wxColor(200, 0, 200),   # purple
	wxColor(255, 255, 0),   # yellow
	wxColor(255, 100, 0),   # orange
	wxColor(255, 150, 130), # pink
	wxColor(139, 115, 85),  # burlywood4
	wxColor(152, 251, 152), # pale green
	wxColor(205, 92, 92),   # indian red
	wxColor(102, 139, 139), # PaleTurquoise4
	wxColor(255, 236, 139), # light goldenrod
	wxColor(159, 154, 98),  # khaki4
	wxColor(149, 81, 147),  # orchid4
	wxColor(0, 139, 69),    # SpringGreen4
]
	

#----------------------------------------------------------------------------

# This is the guts of the configuration script.
# It's simply appended to the file after we write all the configuration options
_ConfigBody = """
import string, sys
sys.path.append( "../server" )
from mothership import *
import tilelayout

# Look for some special mothership params
for (name, value) in MOTHERSHIP_OPTIONS:
	if name == "zeroth_arg":
		ZEROTH_ARG = value
	elif name == "default_dir":
		DEFAULT_DIR = value
	elif name == "auto_start":
		AUTO_START = value
	elif name == "default_app":
		DEFAULT_APP = value

# Check for program name/args on command line
if len(sys.argv) == 1:
	program = DEFAULT_APP
else:
	program = string.join(sys.argv[1:])
if program == "":
	print "No program to run!"
	sys.exit(-1)

# Determine if tiles are on one server or many
if (len(SERVER_HOSTS) >= 2) and (SERVER_HOSTS[0] != SERVER_HOSTS[1]):
	singleServer = 0
else:
	singleServer = 1

localHostname = os.uname()[1]

NumServers = len(TILES) # XXX fix this?


def LayoutTiles(muralWidth, muralHeight):
	if DYNAMIC_SIZE == 1:
		# variable rows/cols, fixed tile size
		tileWidth = TILE_WIDTH
		tileHeight = TILE_HEIGHT
		rows = 0
		cols = 0
	else:
		# fixed rows/cols, variable tile size
		assert DYNAMIC_SIZE == 2
		tileWidth = 0
		tileHeight = 0
		rows = TILE_ROWS
		cols = TILE_COLS

	if LAYOUT == 0:
		tiles = tilelayout.LayoutRaster(muralWidth, muralHeight,
										NumServers,
										tileWidth, tileHeight,
										rows, cols)
	elif LAYOUT == 1:
		tiles = tilelayout.LayoutZigZag(muralWidth, muralHeight,
										NumServers,
										tileWidth, tileHeight,
										rows, cols)
	else:
		tiles = tilelayout.LayoutSpiral(muralWidth, muralHeight,
										NumServers,
										tileWidth, tileHeight,
										rows, cols)
	return tiles



cr = CR()


tilesortSPUs = []
clientNodes = []

for i in range(NUM_APP_NODES):
	tilesortspu = SPU('tilesort')
	for (name, value) in TILESORT_OPTIONS:
		tilesortspu.Conf( name, value )
	if DYNAMIC_SIZE:
		tilesortspu.TileLayoutFunction(LayoutTiles)
		tilesortspu.Conf('optimize_bucket', 0)
	tilesortSPUs.append(tilesortspu)

	clientnode = CRApplicationNode()
	clientnode.AddSPU(tilesortspu)

	# argument substitutions
	if i == 0 and ZEROTH_ARG != "":
		app_string = string.replace( program, '%0', ZEROTH_ARG)
	else:
		app_string = string.replace( program, '%0', '' )
	app_string = string.replace( app_string, '%I', str(i) )
	app_string = string.replace( app_string, '%N', str(NUM_APP_NODES) )
	clientnode.SetApplication( app_string )
	clientnode.StartDir( DEFAULT_DIR )

	if AUTO_START:
		clientnode.AutoStart( ["/bin/sh", "-c",
				"LD_LIBRARY_PATH=%s /usr/local/bin/crappfaker" % crlibdir] )

	clientNodes.append(clientnode)



if REASSEMBLY:
	reassembleNode = CRNetworkNode(REASSEMBLE_HOST)
	reassembleSPU = SPU('render')
	reassembleNode.AddSPU(reassembleSPU)
	for (name, value) in REASSEMBLE_OPTIONS:
		reassembleSPU.Conf( name, value )
	if AUTO_START:
		reassembleNode.AutoStart( ["/usr/bin/rsh", REASSEMBLE_HOST,
								"/bin/sh -c 'DISPLAY=:0.0  CRMOTHERSHIP=%s  LD_LIBRARY_PATH=%s  crserver'" % (REASSEMBLE_HOST, crlibdir) ] )

# Loop over servers
for serverIndex in range(NumServers):

	# Create this server's readback/render SPU
	if REASSEMBLY:
		renderspu = SPU('readback')
	else:
		renderspu = SPU('render')
	for (name, value) in READBACK_OPTIONS:
		renderspu.Conf(name, value)

	# Create network node
	if singleServer:
		host = SERVER_HOSTS[0]
	else:
		host = SERVER_HOSTS[serverIndex]
	servernode = CRNetworkNode(host)

	# Add the tiles
	serverTiles = TILES[serverIndex]
	for tile in serverTiles:
		servernode.AddTile(tile[0], tile[1], tile[2], tile[3])

	# Add SPU to node, node to mothership
	servernode.AddSPU(renderspu)
	if REASSEMBLY:
		packspu = SPU('pack')
		servernode.AddSPU(packspu)
		packspu.AddServer( reassembleNode, protocol='tcpip' )

	for (name, value) in SERVER_OPTIONS:
		servernode.Conf(name, value)
	cr.AddNode(servernode)

	# connect app nodes to server
	for i in range(NUM_APP_NODES):
		tilesortSPUs[i].AddServer(servernode, protocol='tcpip', port=7001+serverIndex)

	# auto-start
	if AUTO_START:
		servernode.AutoStart( ["/usr/bin/rsh", host,
								"/bin/sh -c 'DISPLAY=:0.0  CRMOTHERSHIP=%s  LD_LIBRARY_PATH=%s  crserver'" % (localHostname, crlibdir) ] )


# Add nodes to mothership
for i in range(NUM_APP_NODES):
	cr.AddNode(clientNodes[i])
if REASSEMBLY:
	cr.AddNode(reassembleNode)

# Set mothership params
for (name, value) in MOTHERSHIP_OPTIONS:
	cr.Conf(name, value)

cr.Go()

"""


#----------------------------------------------------------------------------

def FindClientNode(mothership):
	"""Search the mothership for the client node."""
	nodes = mothership.Nodes()
	for n in nodes:
		if n.IsAppNode():
			return n
	return None

def FindServerNode(mothership):
	"""Search the mothership for the server node."""
	client = FindClientNode(mothership)
	assert client != None
	servers = client.GetServers()
	if servers:
		return servers[0]
	else:
		return None

def FindReassemblyNode(mothership):
	"""Search the mothership for the image reassembly node"""
	s = FindServerNode(mothership)
	assert s != None
	servers = s.GetServers()
	assert len(servers) == 0 or len(servers) == 1
	if len(servers) > 0:
		return servers[0]
	else:
		# no reassembly node
		return None

def FindTilesortSPU(mothership):
	"""Search the mothership for the tilesort SPU."""
	appNode = FindClientNode(mothership)
	assert appNode
	tilesortSPU = appNode.LastSPU()
	assert tilesortSPU.Name() == "tilesort"
	return tilesortSPU

def FindReadbackSPU(mothership):
	"""Search the mothership for the readback (or render) SPU."""
	serverNode = FindServerNode(mothership)
	spu = serverNode.SPUChain()[0]
	assert spu.Name() == "render" or spu.Name() == "readback"
	return spu

def FindReassemblySPU(mothership):
	"""Search the mothership for the render SPU."""
	reassemblyNode = FindReassemblyNode(mothership)
	if not reassemblyNode:
		return None
	spus = reassemblyNode.SPUChain()
	if len(spus) == 1 and spus[0].Name() == "render":
		return spus[0]
	else:
		return None


#----------------------------------------------------------------------------

class LightningDialog(wxDialog):
	"""Lightning-2 configuration editor."""

	def __init__(self, parent=NULL, id=-1):
		"""Construct a Lightning-2 dialog."""
		wxDialog.__init__(self, parent, id,
						  title="Image Reassembly Configuration",
						  style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)

		# Widget IDs
		id_MuralWidth  = 5000
		id_MuralHeight = 5001
		id_TileChoice  = 5002
		id_TileWidth   = 5003
		id_TileHeight  = 5004
		id_Layout      = 5005
		id_Dynamic     = 5006
		id_HostText    = 5007
		id_HostStart   = 5008
		id_HostCount   = 5009
		id_OK          = 5010
		id_Cancel      = 5011
		id_NumServers  = 5012
		id_Hostnames   = 5013
		id_Reassembly  = 5014
		id_Help        = 5015

		self.HostNamePattern = "host##"
		self.HostNameStart = 0
		self.HostNameCount = 4

		# this sizer holds all the control widgets
		toolSizer = wxBoxSizer(wxVERTICAL)

		# Rendering node count and names
		box = wxStaticBox(parent=self, id=-1, label="Rendering Nodes",
						  style=wxDOUBLE_BORDER)
		rowSizer = wxStaticBoxSizer(box, wxHORIZONTAL)
		numberLabel = wxStaticText(parent=self, id=-1, label="Number:")
		rowSizer.Add(numberLabel, flag=wxALIGN_CENTER_VERTICAL|wxALL, border=4)
		self.numberControl = wxSpinCtrl(parent=self, id=id_NumServers,
										value="1", min=1, max=10000,
										size=wxSize(50,25))
		EVT_SPINCTRL(self.numberControl, id_NumServers,
					 self.__OnNumServersChange)
		rowSizer.Add(self.numberControl,
					 flag=wxALIGN_CENTER_VERTICAL|wxALL, border=2)
		self.hostsButton = wxButton(parent=self, id=id_Hostnames,
									label=" Host Names... ")
		EVT_BUTTON(self.hostsButton, id_Hostnames, self.__OnHostnames)
		rowSizer.Add(self.hostsButton, flag=wxALIGN_CENTER_VERTICAL|wxLEFT, border=6)
		toolSizer.Add(rowSizer, flag=wxALL|wxGROW, border=2)

		# Dynamic vs Static layout
		dynamicChoices = [ "Fixed size image",
						   "Variable rows/cols, fixed tile size",
						   "Variable tile size, fixed rows/columns" ]
		self.dynamicRadio = wxRadioBox(parent=self, id=id_Dynamic,
									  label="Fixed or Variable Size Image",
									  choices=dynamicChoices,
									  majorDimension=1,
									  style=wxRA_SPECIFY_COLS )
		EVT_RADIOBOX(self.dynamicRadio, id_Dynamic, self.__onDynamicChange)
		toolSizer.Add(self.dynamicRadio, flag=wxEXPAND|wxALL, border=2)

		# Mural width/height (in tiles)
		box = wxStaticBox(parent=self, id=-1, label="Image Tiles",
						  style=wxDOUBLE_BORDER)
		muralSizer = wxStaticBoxSizer(box, wxVERTICAL)
		flexSizer = wxFlexGridSizer(rows=1, cols=4, hgap=4, vgap=4)
		columnsLabel = wxStaticText(parent=self, id=-1,
									label="Columns")
		self.columnsControl = wxSpinCtrl(parent=self, id=id_MuralWidth,
									   value="5", min=1, max=16,
									   size=wxSize(50,25))
		rowsLabel = wxStaticText(parent=self, id=-1,
								 label="Rows:")
		self.rowsControl = wxSpinCtrl(parent=self,
										id=id_MuralHeight,
										value="4", min=1, max=16,
										size=wxSize(50,25))
		EVT_SPINCTRL(self.columnsControl, id_MuralWidth, self.__onSizeChange)
		EVT_SPINCTRL(self.rowsControl, id_MuralHeight, self.__onSizeChange)
		flexSizer.Add(columnsLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.columnsControl)
		flexSizer.Add(rowsLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.rowsControl)
		muralSizer.Add(flexSizer)
		toolSizer.Add(muralSizer, flag=wxEXPAND|wxALL, border=2)

		# Tile size (in pixels)
		box = wxStaticBox(parent=self, id=-1, label="Tile Size",
						  style=wxDOUBLE_BORDER)
		tileSizer = wxStaticBoxSizer(box, wxVERTICAL)
		tileChoices = []
		for i in CommonTileSizes:
			tileChoices.append( str("%d x %d" % (i[0], i[1])) )
		tileChoices.append("Custom")
		self.tileChoice = wxChoice(parent=self, id=id_TileChoice,
								   choices=tileChoices)
		flexSizer = wxFlexGridSizer(rows=1, cols=4, hgap=4, vgap=4)
		self.tileWidthLabel = wxStaticText(parent=self, id=-1,
										   label="Width:")
		self.tileWidthControl = wxSpinCtrl(parent=self,
										   id=id_TileWidth,
										   value="256", min=8, max=2048,
										   size=wxSize(60,25))
		self.tileHeightLabel = wxStaticText(parent=self, id=-1,
											label="Height:")
		self.tileHeightControl = wxSpinCtrl(parent=self,
											id=id_TileHeight,
											value="256", min=8, max=2048,
											size=wxSize(60,25))
		EVT_SPINCTRL(self.tileWidthControl, id_TileWidth, self.__onSizeChange)
		EVT_SPINCTRL(self.tileHeightControl, id_TileHeight, self.__onSizeChange)
		EVT_CHOICE(self.tileChoice, id_TileChoice, self.__onTileChoice)
		flexSizer.Add(self.tileWidthLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.tileWidthControl)
		flexSizer.Add(self.tileHeightLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.tileHeightControl)
#		tileSizer.Add(self.tileChoice, flag=wxALIGN_CENTER|wxALL, border=4)
		tileSizer.Add(self.tileChoice, flag=wxALIGN_LEFT|wxALL, border=4)
		tileSizer.Add(flexSizer)
		toolSizer.Add(tileSizer, flag=wxEXPAND|wxALL, border=2)

		# Total mural size (in pixels)
		box = wxStaticBox(parent=self, id=-1, label="Total Size",
						  style=wxDOUBLE_BORDER)
		totalSizer = wxStaticBoxSizer(box, wxVERTICAL)
		self.totalSizeLabel = wxStaticText(parent=self, id=-1,
										   label="??")
		totalSizer.Add(self.totalSizeLabel, flag=wxEXPAND)
		toolSizer.Add(totalSizer, flag=wxEXPAND|wxALL, border=2)

		# Tile layout
		layoutChoices = [ "Raster order", "Zig-zag raster order",
						  "Spiral from center" ]
		self.layoutRadio = wxRadioBox(parent=self, id=id_Layout,
									  label="Tile Layout",
									  choices=layoutChoices,
									  majorDimension=1,
									  style=wxRA_SPECIFY_COLS )
		EVT_RADIOBOX(self.layoutRadio, id_Layout, self.__onLayoutChange)
		toolSizer.Add(self.layoutRadio, flag=wxEXPAND|wxALL, border=2)

		# Image reassembly
		reassemblyChoices = [ "No reassembly",
							  "Reassemble into render SPU window",
							  "Reassemble into application window" ]
		self.reassemblyRadio = wxRadioBox(parent=self, id=id_Layout,
									  label="Image Reassembly",
									  choices=reassemblyChoices,
									  majorDimension=1,
									  style=wxRA_SPECIFY_COLS )
		EVT_RADIOBOX(self.reassemblyRadio, id_Reassembly, self.__onReassemblyChange)
		toolSizer.Add(self.reassemblyRadio, flag=wxEXPAND|wxALL, border=2)

		# Setup the drawing area
		self.drawArea = wxPanel(self, id=-1, style=wxSUNKEN_BORDER)
		self.drawArea.SetBackgroundColour(BackgroundColor)
		EVT_PAINT(self.drawArea, self.__onPaintEvent)

		# Sizer for the OK, Cancel buttons
		okCancelSizer = wxGridSizer(rows=1, cols=3, vgap=4, hgap=20)
		self.OkButton = wxButton(parent=self, id=id_OK, label="OK")
		okCancelSizer.Add(self.OkButton, option=0,
						  flag=wxALIGN_CENTER, border=0)
		self.CancelButton = wxButton(parent=self, id=id_Cancel, label="Cancel")
		okCancelSizer.Add(self.CancelButton, option=0,
						  flag=wxALIGN_CENTER, border=0)
		self.HelpButton = wxButton(parent=self, id=id_Help, label="Help")
		okCancelSizer.Add(self.HelpButton, option=0,
						  flag=wxALIGN_CENTER, border=0)

		EVT_BUTTON(self.OkButton, id_OK, self._onOK)
		EVT_BUTTON(self.CancelButton, id_Cancel, self._onCancel)
		EVT_BUTTON(self.HelpButton, id_Help, self._onHelp)

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
		topSizer.Add(toolAndDrawSizer, option=1, flag=wxGROW, border=0)
		topSizer.Add(separator, flag=wxGROW, border=0)
		topSizer.Add(okCancelSizer, option=0, flag=wxGROW|wxALL, border=10)

		# Finish-up the dialog
		self.SetAutoLayout(true)
		self.SetSizer(topSizer)

		minSize = topSizer.GetMinSize()
		minSize[0] = 600
		minSize[1] += 10
		self.SetSizeHints(minW=750, minH=minSize[1])
		self.SetSize(minSize)

		# Hostname dialog
		self.hostsDialog = hostdialog.HostDialog(parent=NULL, id=-1,
						title="Chromium Hosts",
						message="Specify host names for the render nodes")
		self.hostsDialog.Centre()

	# end of __init__()

	def __UpdateDependentWidgets(self):
		"""Update tile choice widget and the total mural size readout."""
		w = self.tileWidthControl.GetValue()
		h = self.tileHeightControl.GetValue()
		assert w == self.Template.TileWidth
		assert h == self.Template.TileHeight
		for i in range(0, len(CommonTileSizes)):
			if w == CommonTileSizes[i][0] and h == CommonTileSizes[i][1]:
				self.tileChoice.SetSelection(i)
				break
		if i >= len(CommonTileSizes):
			# must be custom size
			self.tileChoice.SetSelection(len(CommonTileSizes))  # "Custom"
		# dynamic = 0 -> fixed
		dynamic = self.dynamicRadio.GetSelection()
		# update total mural size readout
		if dynamic == 0:
			totalW = self.Template.Columns * self.Template.TileWidth
			totalH = self.Template.Rows * self.Template.TileHeight
			self.totalSizeLabel.SetLabel(str("%d x %d" % (totalW, totalH)))
		else:
			self.totalSizeLabel.SetLabel("Variable")
		if dynamic == 0:
			self.columnsControl.Enable(TRUE)
			self.rowsControl.Enable(TRUE)
			self.tileChoice.Enable(TRUE)
			self.tileWidthControl.Enable(TRUE)
			self.tileHeightControl.Enable(TRUE)
		elif dynamic == 1:
			self.columnsControl.Enable(FALSE)
			self.rowsControl.Enable(FALSE)
			self.tileChoice.Enable(TRUE)
			self.tileWidthControl.Enable(TRUE)
			self.tileHeightControl.Enable(TRUE)
		else:
			assert dynamic == 2
			self.columnsControl.Enable(TRUE)
			self.rowsControl.Enable(TRUE)
			self.tileChoice.Enable(FALSE)
			self.tileWidthControl.Enable(FALSE)
			self.tileHeightControl.Enable(FALSE)


	def __UpdateWidgetsFromVars(self):
		"""Update the widgets from template values."""
		self.numberControl.SetValue(self.Template.NumServers)
		self.columnsControl.SetValue(self.Template.Columns)
		self.rowsControl.SetValue(self.Template.Rows)
		self.tileWidthControl.SetValue(self.Template.TileWidth)
		self.tileHeightControl.SetValue(self.Template.TileHeight)
		self.layoutRadio.SetSelection(self.Template.Layout)
		self.dynamicRadio.SetSelection(self.Template.DynamicSize)
		self.reassemblyRadio.SetSelection(self.Template.Reassembly)

	def __UpdateVarsFromWidgets(self):
		"""Update the template values from the widgets."""
		self.Template.NumServers = self.numberControl.GetValue()
		self.Template.Rows = self.rowsControl.GetValue()
		self.Template.Columns = self.columnsControl.GetValue()
		self.Template.TileWidth = self.tileWidthControl.GetValue()
		self.Template.TileHeight = self.tileHeightControl.GetValue()
		self.Template.Layout = self.layoutRadio.GetSelection()
		self.Template.DynamicSize = self.dynamicRadio.GetSelection()
		self.Template.Reassembly = self.reassemblyRadio.GetSelection()


	#----------------------------------------------------------------------
	# Widget callback functions

	def __OnNumServersChange(self, event):
		"Called when number of servers changes"""
		self.__UpdateVarsFromWidgets()
		if len(self.Template.ServerHosts) < self.Template.NumServers:
			# generate additional host names
			k = len(self.Template.ServerHosts)
			n = self.Template.NumServers - k
			start = self.Template.ServerPattern[1] + k
			newHosts = crutils.MakeHostnames(self.Template.ServerPattern[0],
											 start, n)
			self.Template.ServerHosts += newHosts
		self.Template.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	def __OnHostnames(self, event):
		"""Called when the hostnames button is pressed."""
		self.hostsDialog.SetHosts(self.Template.ServerHosts)
		self.hostsDialog.SetCount(self.Template.NumServers)
		self.hostsDialog.SetHostPattern(self.Template.ServerPattern)
		if self.hostsDialog.ShowModal() == wxID_OK:
			self.Template.ServerHosts = self.hostsDialog.GetHosts()
			self.Template.NumServers = self.hostsDialog.GetCount()
			self.Template.ServerPattern = self.hostsDialog.GetHostPattern()
		self.drawArea.Refresh()
		self.dirty = true

	def __onSizeChange(self, event):
		"""Called when tile size changes with spin controls."""
		self.__UpdateVarsFromWidgets()
		self.__UpdateDependentWidgets()
		self.Template.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	def __onTileChoice(self, event):
		"""Called when tile size changes with combo-box control."""
		i = self.tileChoice.GetSelection()
		if i < len(CommonTileSizes):
			w = CommonTileSizes[i][0]
			h = CommonTileSizes[i][1]
			self.tileWidthControl.SetValue(w)
			self.tileHeightControl.SetValue(h)
		self.__UpdateVarsFromWidgets()
		self.__UpdateDependentWidgets()
		self.Template.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	def __onLayoutChange(self, event):
		"""Called when Layout order changes."""
		self.__UpdateVarsFromWidgets()
		self.Template.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	def __onDynamicChange(self, event):
		"""Called when Dynamic vs Static layout changes."""
		self.__UpdateVarsFromWidgets()
		self.__UpdateDependentWidgets()
		#self.Template.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	def __onReassemblyChange(self, event):
		"""Called when image reassembly option changes."""
		self.__UpdateVarsFromWidgets()
		#self.Template.LayoutTiles()
		#self.drawArea.Refresh()
		self.dirty = true


	def _onOK(self, event):
		"""Called by OK button"""
		# xxx OK vs cancel updates?
		#self.__UpdateVarsFromWidgets()
		self.EndModal(wxID_OK)

	def _onCancel(self, event):
		"""Called by Cancel button"""
		# xxx OK vs cancel updates?
		self.EndModal(wxID_CANCEL)

	def _onHelp(self, event):
		"""Called by Help button"""
		d = textdialog.TextDialog(parent=self, id = -1,
								  title="Image Reassembly Help")
		#d.SetPage("Image reassembly help coming soon")
		d.LoadPage("../../doc/reassembly_template.html")
		# Hmmm, I want to call d.Show() so this isn't modal/blocking but
		# that doesn't seem to work.
		d.ShowModal();
		d.Destroy()

	def __onPaintEvent(self, event):
		""" Respond to a request to redraw the contents of our drawing panel.
		"""
		# border around the window and space between the tiles
		border = 20
		space = 2

		dc = wxPaintDC(self.drawArea)
		dc.BeginDrawing()
		dc.SetPen(wxBLACK_PEN);
		dc.SetBrush(wxLIGHT_GREY_BRUSH);
		
		# Get current settings
		size = self.drawArea.GetSize()
		cols = self.Template.Columns
		rows = self.Template.Rows
		tileWidth = self.Template.TileWidth
		tileHeight = self.Template.TileHeight

		# how many pixels we'd like to draw
		desiredWidth = cols * tileWidth
		desiredHeight = rows * tileHeight

		# how much space we have in the window
		availWidth = size.width - 2 * border - (cols + 1) * space
		availHeight = size.height - 2 * border - (rows + 1) * space

		# Compute scale factor so we fill the drawing area with the
		# screen diagram, preserving the tile and screen aspect ratios.
		xscale = float(availWidth) / float(desiredWidth)
		yscale = float(availHeight) / float(desiredHeight)

		if xscale < yscale:
			scale = xscale
		else:
			scale = yscale

		# draw the tiles as boxes
		hosts = self.Template.ServerHosts
		numColors = len(ServerColors)
		numServers = self.Template.NumServers
		for (server, x, y, w, h) in self.Template.ServerTiles:
			# apply scale factor, bias to fit mural into the drawing area
			x = x * scale + border
			y = y * scale + border
			w *= scale
			h *= scale

			color = server % numColors
			dc.SetBrush(wxBrush(ServerColors[color]))
			dc.DrawRectangle(x, y, w,h)
			if server < len(hosts):
				s = hosts[server]
			else:
				s = hosts[-1]
			(tw, th) = dc.GetTextExtent(s)
			dx = (w - tw) / 2
			dy = (h - th) / 2
			dc.DrawText(s, x+dx, y+dy)

		# draw top width label
		dynamic = self.dynamicRadio.GetSelection()
		if dynamic == 0:
			topLabel = "<---  %d  --->" % desiredWidth
			(tw, th) = dc.GetTextExtent(topLabel)
			dc.DrawText(topLabel, (size.width - tw) / 2, 2)

			# draw left height label (very crude for now)
			leftLabel = "^|| %d ||v" % desiredHeight
			(tw, th) = dc.GetTextExtent(leftLabel[0:1])
			totalH = th * len(leftLabel)
			x = 6
			y = (size.height - totalH) / 2
			for i in range(0, len(leftLabel)):
				dc.DrawText(leftLabel[i:i+1], x, y)
				y += th

		dc.EndDrawing()


	def ShowModal(self, mothership):
		"""Show the dialog and block until OK or Cancel is chosen."""
		# Load the template values
		self.Template = mothership.Template.Clone()
		self.Template.UpdateFromMothership(mothership)
		self.__UpdateWidgetsFromVars()
		self.__UpdateDependentWidgets()
		self.Template.LayoutTiles()
		# show the dialog
		retVal = wxDialog.ShowModal(self)
		if retVal == wxID_OK:
			# update the template vars and mothership
			self.__UpdateVarsFromWidgets()
			self.Template.UpdateMothership(mothership)
			mothership.Template = self.Template
		return retVal



#----------------------------------------------------------------------

class ReassemblyTemplate(templatebase.TemplateBase):
	"""Template for creating/editing/reading/writing image reassembly configs."""
	def __init__(self):
		# no-op for now
		pass
		
	def Name(self):
		return "Image Reassembly"

	def Create(self, parentWindow, mothership):
		"""Create nodes/spus/etc for image reassembly config."""
		defaultMuralSize = crutils.GetSiteDefault("mural_size")
		if defaultMuralSize:
			dialogDefaults = [ 1, defaultMuralSize[0], defaultMuralSize[1] ]
		else:
			dialogDefaults = [1, 2, 1]

		# XXX also prompt for tile size here?
		dialog = intdialog.IntDialog(parent=parentWindow, id=-1,
									 title="Image Reassembly Template",
									 labels=["Number of application nodes:",
											 "Number of server nodes:",
											 "Number of Columns:",
											 "Number of Rows:"],
									 defaultValues=[1, 4, 5, 4],
									 minValue=1, maxValue=10000)
		dialog.Centre()
		if dialog.ShowModal() == wxID_CANCEL:
			dialog.Destroy()
			return 0

		# Init parameters
		values = dialog.GetValues()
		numClients = values[0]
		numServers = values[1]
		cols = values[2]
		rows = values[3]
		mothership.Template = LightningParameters(rows, cols)

		hosts = crutils.GetSiteDefault("cluster_hosts")
		if hosts:
			mothership.Template.ServerHosts = hosts
		tileSize = crutils.GetSiteDefault("tile_size")
		if tileSize:
			mothership.Template.TileWidth = tileSize[0]
			mothership.Template.TileHeight = tileSize[1]
		screenSize = crutils.GetSiteDefault("screen_size")
		if screenSize:
			mothership.Template.ScreenWidth = screenSize[0]
			mothership.Template.ScreenHeight = screenSize[1]

		mothership.Template.LayoutTiles()  # initial tile layout

		# build the graph
		mothership.DeselectAllNodes()
		# Create the <numClients> app nodes
		appNode = crutils.NewApplicationNode(numClients)
		appNode.SetPosition(20, 80)
		appNode.Select()
		tilesortSPU = crutils.NewSPU("tilesort")
		appNode.AddSPU(tilesortSPU)
		mothership.AddNode(appNode)
		# Create the <numServers> server nodes
		serverNode = crutils.NewNetworkNode(numServers)
		serverNode.SetPosition(210, 80)
		serverNode.Select()
		readbackSPU = crutils.NewSPU("readback")
		readbackSPU.SetOption('window_geometry', [0, 0, 400, 400])  # XXX fix me
		readbackSPU.SetOption('title', ["Chromium Readback SPU"])
		packSPU = crutils.NewSPU("pack")
		serverNode.AddSPU(readbackSPU)
		serverNode.AddSPU(packSPU)
		mothership.AddNode(serverNode)
		tilesortSPU.AddServer(serverNode)
		# Create the tile reassembly node
		hosts = crutils.GetSiteDefault("frontend_hosts")
		if not hosts:
			hosts = ["localhost"]
		reassemblyNode = crtypes.NetworkNode([ hosts[0] ], 1)
		reassemblyNode.SetPosition(420, 80)
		reassemblyNode.Select()
		reassemblySPU = crutils.NewSPU('render')
		reassemblyNode.AddSPU(reassemblySPU)
		mothership.AddNode(reassemblyNode)
		packSPU.AddServer(reassemblyNode)

		# done with the dialog
		dialog.Destroy()
		return 1

	def Validate(self, mothership):
		"""Test if the mothership config is an image reassembly config."""
		# First, check for client node and tilesort spu
		clientNode = FindClientNode(mothership)
		if not clientNode:
			return 0
		tilesortSPU = FindTilesortSPU(mothership)
		if not tilesortSPU:
			return 0
		# check for render/readback node
		serverNode = FindServerNode(mothership)
		if not serverNode:
			return 0
		serverSPUs = serverNode.SPUChain()
		if len(serverSPUs) != 1 and len(serverSPUs) != 2:
			return 0
		if serverSPUs[0].Name() != "render" and serverSPUs[0].Name() != "readback":
			return 0
		if len(serverSPUs) > 1 and serverSPUs[1].Name() != "pack":
			return 0
		# check for reassembly node
		reassemblyNode = FindReassemblyNode(mothership)
		if reassemblyNode:
			spus = reassemblyNode.SPUChain()
			if len(spus) != 1:
				return 0
			if spus[0].Name() != "render":
				return 0
		# OK, this is a good config!
		return 1

	def Edit(self, parentWindow, mothership):
		"""Open editor window and edit the mothership config"""
		if not self.Validate(mothership):
			print "This is not a Lightning-2 configuration!"
			return

		# XXX we only need to create one instance of the LightningDialog() and
		# reuse it in the future.
		dialog = LightningDialog(parent=parentWindow)
		dialog.Centre()
		retVal = dialog.ShowModal(mothership)
		dialog.Destroy()
		return retVal

	def Read(self, mothership, fileHandle):
		"""Read image reassembly config from file"""
		mothership.Template = LightningParameters()

		# build the nodes and SPUs
		reassembleNode = crtypes.NetworkNode()
		reassembleSPU = crutils.NewSPU("render")
		reassembleNode.AddSPU(reassembleSPU)

		serverNode = crtypes.NetworkNode()
		readbackSPU = crutils.NewSPU("readback")
		packSPU = crutils.NewSPU("pack")
		packSPU.AddServer(reassembleNode)
		serverNode.AddSPU(readbackSPU)
		serverNode.AddSPU(packSPU)

		clientNode = crtypes.ApplicationNode()
		tilesortSPU = crutils.NewSPU("tilesort")
		tilesortSPU.AddServer(serverNode)
		clientNode.AddSPU(tilesortSPU)

		mothership.AddNode(clientNode)
		mothership.AddNode(serverNode)
		mothership.AddNode(reassembleNode)

		numClients = 1
		numServers = 1
		reassembly = 0

		# useful regex patterns
		integerPat = "[0-9]+"
		listPat = "\[.+\]"
		tuplePat = "\(.+\)"

		while true:
			l = fileHandle.readline()
			if not l:
				break
			# remove trailing newline character
			if l[-1:] == '\n':
				l = l[:-1]
			if re.match("^NUM_SERVERS = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				numServers = int(l[v.start() : v.end()])
			elif re.match("^TILE_ROWS = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				mothership.Template.Rows = int(l[v.start() : v.end()])
			elif re.match("^TILE_COLS = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				mothership.Template.Columns = int(l[v.start() : v.end()])
			elif re.match("^TILE_WIDTH = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				mothership.Template.TileWidth = int(l[v.start() : v.end()])
			elif re.match("^TILE_HEIGHT = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				mothership.Template.TileHeight = int(l[v.start() : v.end()])
			elif re.match("^LAYOUT = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				mothership.Template.Layout = int(l[v.start() : v.end()])
			elif re.match("^SERVER_HOSTS = ", l):
				v = re.search(listPat + "$", l)
				hosts = eval(l[v.start() : v.end()])
				serverNode.SetHosts(hosts)
			elif re.match("^SERVER_PATTERN = ", l):
				v = re.search(tuplePat + "$", l)
				pattern = eval(l[v.start() : v.end()])
				serverNode.SetHostNamePattern(pattern)
			elif re.match("^NUM_APP_NODES = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				numClients = int(l[v.start() : v.end()])
			elif re.match("^REASSEMBLY = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				mothership.Template.Reassembly = int(l[v.start() : v.end()])
			elif re.match("^DYNAMIC_SIZE = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				mothership.Template.DynamicSize = int(l[v.start() : v.end()])
			elif re.match("^TILESORT_OPTIONS = \[", l):
				tilesortSPU.GetOptions().Read(fileHandle)
			elif re.match("^REASSEMBLE_OPTIONS = \[", l):
				reassembleSPU.GetOptions().Read(fileHandle)
			elif re.match("^READBACK_OPTIONS = \[", l):
				readbackSPU.GetOptions().Read(fileHandle)
			elif re.match("^SERVER_OPTIONS = \[", l):
				serverNode.GetOptions().Read(fileHandle)
			elif re.match("^MOTHERSHIP_OPTIONS = \[", l):
				mothership.GetOptions().Read(fileHandle)
			elif re.match("^# end of options", l):
				# that's the end of the variables
				# save the rest of the file....
				break
			elif (l != "") and (not re.match("\s*#", l)):
				print "unrecognized line: %s" % l
		# endwhile

		clientNode.SetCount(numClients)
		serverNode.SetCount(numServers)

		#if reassembly:
		#	reassemblyNode = crtypes.NetworkNode('localhost')  # XXX host OK?
		#	reassemblySPU = crutils.NewSPU('render')
		#	mothership.AddNode(reassemblyNode)

		mothership.LayoutNodes()
		return 1

	def Write(self, mothership, fileHandle):
		"""Write a image reassembly config to the given file handle."""
		if not self.Validate(mothership):
			return 0

		template = mothership.Template
		template.UpdateFromMothership(mothership)
		template.LayoutTiles() # just in case this wasn't done already

		clientNode = FindClientNode(mothership)
		serverNode = FindServerNode(mothership)
		reassembleNode = FindReassemblyNode(mothership)

		file = fileHandle
		file.write('TEMPLATE = "%s"\n' % self.Name())
		file.write("NUM_SERVERS = %d\n" % template.NumServers)
		file.write("TILE_ROWS = %d\n" % template.Rows)
		file.write("TILE_COLS = %d\n" % template.Columns)
		file.write("TILE_WIDTH = %d\n" % template.TileWidth)
		file.write("TILE_HEIGHT = %d\n" % template.TileHeight)
		file.write("LAYOUT = %d\n" % template.Layout)
		file.write("SERVER_HOSTS = %s\n" % str(serverNode.GetHosts()))
		file.write('SERVER_PATTERN = %s\n' % str(serverNode.GetHostNamePattern()))
		file.write("NUM_APP_NODES = %d\n" % clientNode.GetCount())
		file.write("DYNAMIC_SIZE = %d\n" % template.DynamicSize)
		file.write("REASSEMBLY = %d\n" % template.Reassembly)
		if reassembleNode:
			host = reassembleNode.GetHosts()[0]
		else:
			host = "localhost"
		file.write('REASSEMBLE_HOST = "%s"\n' % host)

		# write tilesort SPU options
		tilesortSPU = FindTilesortSPU(mothership)
		tilesortSPU.GetOptions().Write(file, "TILESORT_OPTIONS")

		# write render SPU options
		readbackSPU = FindReadbackSPU(mothership)
		readbackSPU.GetOptions().Write(file, "READBACK_OPTIONS")

		# write reassemble SPU options
		reassemblySPU = FindReassemblySPU(mothership)
		reassemblySPU.GetOptions().Write(file, "REASSEMBLE_OPTIONS")


		# write server and global options
		serverNode.GetOptions().Write(file, "SERVER_OPTIONS")
		mothership.GetOptions().Write(file, "MOTHERSHIP_OPTIONS")

		file.write("# end of options, the rest is boilerplate\n")

		# The tiles will be recomputed when we reload the config file so
		# we can put this info after the parameter sectino.
		file.write("\n")
		file.write("TILES = %s\n" % str(template.MuralTiles))

		file.write(_ConfigBody)
		return 1

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



import string, cPickle, os.path, re
from wxPython.wx import *
import crtypes, crutils, intdialog, hostdialog, configio


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
		self.Reassembly = 0

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
		
	def __AllocTile(self, server, row, col):
		"""Allocate a tile for mural position (row, col) on the nth server.
		Return 1 for success, 0 if we run out of room on the server."""
		# XXX we're assuming all tiles are the same size here
		assert server >= 0
		assert server < self.NumServers
		assert self.NumServers == len(self.ServerTiles)
		assert len(self.ServerTiles) == len(self.NextTile)
		# Check if tile of size (TileWidth, TileHeight) will fit on this server
		# We allocate tiles in raster order, as in the crserver.
		(x, y) = self.NextTile[server] # (x,y) position on server's screen
		if y + self.TileHeight > self.ScreenHeight:
			# ran out of room on this server!!!
			return 0
		elif x + self.TileWidth > self.ScreenWidth:
			# to to next row
			x = 0
			y += self.TileHeight
			if y + self.TileHeight > self.ScreenHeight:
				# ran out of room on this server!!!
				return 0
		# It'll fit, save it
		mx = col * self.TileWidth   # mural X coord
		my = row * self.TileHeight  # mural Y coord
		muralTile = (mx, my, self.TileWidth, self.TileHeight)
		self.ServerTiles[server].append( muralTile )
		self.Tiles.append( (row, col, server) )
		# Update NextTile position for this server
		x += self.TileWidth
		self.NextTile[server] = (x, y)
		return 1

	def PrintTiles(self):
		# for debug only
		for i in range(len(self.ServerTiles)):
			print "server %d" % i
			for tile in self.ServerTiles[i]:
				print "  (%d, %d, %d, %d)" % tile

	def LayoutTiles(self):
		"""Compute locations and hosts for the tiles."""

		# initialize tile lists
		self.ServerTiles = []  # array [server] of array of (x,y,w,h)
		self.Tiles = []        # tuples (row, col, server) for drawing
		self.NextTile = []     # array [server] of (row,col)
		for i in range(self.NumServers):
			self.ServerTiles.append( [] )
			self.NextTile.append( (0, 0) )

		# begin layout
		if self.Layout == 0:
			# Simple raster order layout
			for i in range(self.Rows):
				for j in range(self.Columns):
					server = (i * self.Columns + j) % self.NumServers
					self.__AllocTile(server, i, j)
			#endfor
		elif self.Layout == 1:
			# Slightly different raster order layout
			for i in range(self.Rows):
				for j in range(self.Columns):
					if i % 2 == 1:
						# odd row
						server = (i * self.Columns + (self.Columns - j - 1)) % self.NumServers
					else:
						# even row
						server = (i * self.Columns + j) % self.NumServers
					self.__AllocTile(server, i, j)
			#endfor
		else:
			# Spiral outward from the center (this is a little tricky)
			assert self.Layout == 2
			curRow = (self.Rows - 1) / 2
			curCol = (self.Columns - 1) / 2
			radius = 0
			march = 0
			colStep = 0
			rowStep = -1
			serv = 0
			while 1:
				assert ((rowStep == 0 and colStep != 0) or
						(rowStep != 0 and colStep == 0))
				if (curRow >= 0 and curRow < self.Rows and
					curCol >= 0 and	curCol < self.Columns):
					# save this tile location
					#server = len(self.Tiles) % self.NumServers
					server = serv % self.NumServers
					assert (curRow, curCol, server) not in self.Tiles
					if not self.__AllocTile(server, curRow, curCol):
						outOfSpace = 1
					else:
						outOfSpace = 0
					# check if we're done
					if ((len(self.Tiles) >= self.Rows * self.Columns) or
						outOfSpace):
						# all done
						break
				serv += 1
				# advance to next space
				march += 1
				if march < radius:
					# step in current direction
					curRow += rowStep
					curCol += colStep
					pass
				else:
					# change direction
					if colStep == 1 and rowStep == 0:
						# transition right -> down
						colStep = 0
						rowStep = 1
					elif colStep == 0 and rowStep == 1:
						# transition down -> left
						colStep = -1
						rowStep = 0
						radius += 1
					elif colStep == -1 and rowStep == 0:
						# transition left -> up
						colStep = 0
						rowStep = -1
					else:
						# transition up -> right
						assert colStep == 0
						assert rowStep == -1
						colStep = 1
						rowStep = 0
						radius += 1
					#endif
					march = 0
					curRow += rowStep
					curCol += colStep
				#endif
			#endwhile
		#endif
	#enddef



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

# Determine if tiles are on one server or many
if (len(SERVER_HOSTS) >= 2) and (SERVER_HOSTS[0] != SERVER_HOSTS[1]):
	singleServer = 0
else:
	singleServer = 1

localHostname = os.uname()[1]

if REASSEMBLY == 2:
	RENDER_TO_APP_WINDOW = 1
else:
	RENDER_TO_APP_WINDOW = 0

cr = CR()
cr.MTU( GLOBAL_MTU )


tilesortSPUs = []
clientNodes = []

for i in range(NUM_APP_NODES):
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

	# argument substitutions
	if i == 0 and GLOBAL_zeroth_arg != "":
		app_string = string.replace( program, '%0', GLOBAL_zeroth_arg)
	else:
		app_string = string.replace( program, '%0', '' )
	app_string = string.replace( app_string, '%I', str(i) )
	app_string = string.replace( app_string, '%N', str(NUM_APP_NODES) )
	clientnode.SetApplication( app_string )
	clientnode.StartDir( GLOBAL_default_dir )

	if GLOBAL_auto_start:
		clientnode.AutoStart( ["/bin/sh", "-c",
				"LD_LIBRARY_PATH=%s /usr/local/bin/crappfaker" % crlibdir] )

	clientNodes.append(clientnode)


NumServers = len(TILES)
SCREEN_HEIGHT = 1280
SCREEN_WIDTH = 1024

if REASSEMBLY:
	reassembleNode = CRNetworkNode()  # xxx host?
	reassembleSPU = SPU('render')
	reassembleNode.AddSPU(reassembleSPU)
	# xxx reassembleSPU.Conf()
	if RENDER_TO_APP_WINDOW:
		reassembleSPU.Conf('render_to_app_window', '1')
	else:
		reassembleSPU.Conf('window_geometry',
								REASSEMBLE_window_geometry[0],
								REASSEMBLE_window_geometry[1],
								REASSEMBLE_window_geometry[2],
								REASSEMBLE_window_geometry[3])

# Loop over servers
for serverIndex in range(NumServers):

	# Create this server's render SPU
	if REASSEMBLY:
		renderspu = SPU('readback')
	else:
		renderspu = SPU('render')
	renderspu.Conf('try_direct', READBACK_try_direct)
	renderspu.Conf('force_direct', READBACK_force_direct)
	renderspu.Conf('fullscreen', READBACK_fullscreen)
	renderspu.Conf('title', READBACK_title)
	renderspu.Conf('system_gl_path', READBACK_system_gl_path)

	# Setup render SPU's window geometry
	renderspu.Conf('window_geometry',
					READBACK_window_geometry[0],
					READBACK_window_geometry[1],
					READBACK_window_geometry[2],
					READBACK_window_geometry[3] )
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
		packspu.AddServer( reassembleNode, protocol='tcpip', port=7777 )

	servernode.Conf('optimize_bucket', SERVER_optimize_bucket)
	cr.AddNode(servernode)

	# connect app nodes to server
	for i in range(NUM_APP_NODES):
		tilesortSPUs[i].AddServer(servernode, protocol='tcpip', port=7000+serverIndex)

	# auto-start
	if GLOBAL_auto_start:
		servernode.AutoStart( ["/usr/bin/rsh", host,
								"/bin/sh -c 'DISPLAY=:0.0  CRMOTHERSHIP=%s  LD_LIBRARY_PATH=%s  crserver'" % (localHostname, crlibdir) ] )


for i in range(NUM_APP_NODES):
	cr.AddNode(clientNodes[i])
if REASSEMBLY:
	cr.AddNode(reassembleNode)

cr.SetParam('minimum_window_size', GLOBAL_minimum_window_size)
cr.SetParam('match_window_title', GLOBAL_match_window_title)
cr.SetParam('show_cursor', GLOBAL_show_cursor)
cr.SetParam('track_window_size', DYNAMIC_SIZE)
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
	# XXX avoid the reassembly node
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
						   "Variable size, fixed tile size",
						   "Variable size, fixed rows/columns" ]
		self.dynamicRadio = wxRadioBox(parent=self, id=id_Dynamic,
									  label="Fixed or Variable Size Image",
									  choices=dynamicChoices,
									  majorDimension=1,
									  style=wxRA_SPECIFY_COLS )
		EVT_RADIOBOX(self.dynamicRadio, id_Dynamic, self.__onDynamicChange)
		toolSizer.Add(self.dynamicRadio, flag=wxEXPAND|wxALL, border=2)

		# Mural width/height (in tiles)
		box = wxStaticBox(parent=self, id=-1, label="Tiling Size",
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
		okCancelSizer = wxGridSizer(rows=1, cols=2, vgap=4, hgap=20)
		self.OkButton = wxButton(parent=self, id=id_OK, label="OK")
		okCancelSizer.Add(self.OkButton, option=0,
						  flag=wxALIGN_CENTER, border=0)
		self.CancelButton = wxButton(parent=self, id=id_Cancel,
									 label="Cancel")
		okCancelSizer.Add(self.CancelButton, option=0,
						  flag=wxALIGN_CENTER, border=0)
		EVT_BUTTON(self.OkButton, id_OK, self._onOK)
		EVT_BUTTON(self.CancelButton, id_Cancel, self._onCancel)

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
		"""Update the widgets from internal vars."""
		self.numberControl.SetValue(self.Template.NumServers)
		self.columnsControl.SetValue(self.Template.Columns)
		self.rowsControl.SetValue(self.Template.Rows)
		self.tileWidthControl.SetValue(self.Template.TileWidth)
		self.tileHeightControl.SetValue(self.Template.TileHeight)
		self.layoutRadio.SetSelection(self.Template.Layout)
		self.dynamicRadio.SetSelection(self.Template.DynamicSize)
		self.reassemblyRadio.SetSelection(self.Template.Reassembly)

	def __UpdateVarsFromWidgets(self):
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

		w = tileWidth * scale
		h = tileHeight * scale

		# draw the tiles as boxes
		hosts = self.Template.ServerHosts
		numColors = len(ServerColors)
		numServers = self.Template.NumServers
		for (row, col, server) in self.Template.Tiles:
			x = col * (w + space) + border
			y = row * (h + space) + border
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
			mothership.Template = self.Template
			clientNode = FindClientNode(mothership)
			serverNode = FindServerNode(mothership)
			serverNode.SetCount(mothership.Template.NumServers)
			serverNode.SetHostNamePattern(mothership.Template.ServerPattern)
			serverNode.SetHosts(mothership.Template.ServerHosts)
			for i in range(serverNode.GetCount()):
				serverNode.SetTiles(mothership.Template.ServerTiles[i], i)
		return retVal


def Create_Lightning2(parentWindow, mothership):
	"""Create a Lightning2- configuration"""
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
	appNode.SetPosition(10, 50)
	appNode.Select()
	tilesortSPU = crutils.NewSPU("tilesort")
	appNode.AddSPU(tilesortSPU)
	mothership.AddNode(appNode)
	# Create the <numServers> server nodes
	serverNode = crutils.NewNetworkNode(numServers)
	serverNode.SetPosition(210, 50)
	serverNode.Select()
	readbackSPU = crutils.NewSPU("readback")
	packSPU = crutils.NewSPU("pack")
	serverNode.AddSPU(readbackSPU)
	serverNode.AddSPU(packSPU)
	mothership.AddNode(serverNode)
	tilesortSPU.AddServer(serverNode)
	# Create the tile reassembly node
	reassemblyNode = crutils.NewNetworkNode(1)
	reassemblyNode.SetPosition(420, 50)
	reassemblyNode.Select()
	reassemblySPU = crutils.NewSPU('render')
	reassemblyNode.AddSPU(reassemblySPU)
	mothership.AddNode(reassemblyNode)
	packSPU.AddServer(reassemblyNode)

	# done with the dialog
	dialog.Destroy()
	return 1


def Is_Lightning2(mothership):
	"""Test if the mothership describes a lightning-2 configuration.
	Return 1 if so, 0 otherwise."""
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
	# OK, this is a tilesort config!
	return 1


def Edit_Lightning2(parentWindow, mothership):
	"""Edit parameters for a Lightning2 template"""
	t = Is_Lightning2(mothership)
	if not t:
		print "This is not a Lightning-2 configuration!"
		return

	print "Edit lightning-2"

	# XXX we only need to create one instance of the LightningDialog() and
	# reuse it in the future.
	dialog = LightningDialog(parent=parentWindow)
	dialog.Centre()
	retVal = dialog.ShowModal(mothership)
	dialog.Destroy()
	return retVal


def Read_Lightning2(mothership, fileHandle):
	"""Read a Lightning-2 config from the given file handle."""
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
			
		elif re.match("^TILESORT_", l):
			# A tilesort SPU option
			(name, values) = configio.ParseOption(l, "TILESORT")
			tilesortSPU.SetOption(name, values)
		elif re.match("^REASSEMBLE_", l):
			# A render SPU option
			(name, values) = configio.ParseOption(l, "REASSEMBLE")
			reassembleSPU.SetOption(name, values)
		elif re.match("^READBACK_", l):
			# A readback SPU option
			(name, values) = configio.ParseOption(l, "READBACK")
			readbackSPU.SetOption(name, values)

		elif re.match("^SERVER_", l):
			# A server option
			(name, values) = configio.ParseOption(l, "SERVER")
			serverNode.SetOption(name, values)
		elif re.match("^GLOBAL_", l):
			# A global option
			(name, values) = configio.ParseOption(l, "GLOBAL")
			mothership.SetOption(name, values)
		elif re.match("^# end of options", l):
			# that's the end of the variables
			# save the rest of the file....
			break
		elif (l != "") and (not re.match("\s*#", l)):
			print "unrecognized line: %s" % l
	# endwhile

	clientNode.SetCount(numClients)
	serverNode.SetCount(numServers)

#	if reassembly:
#		reassemblyNode = crtypes.NetworkNode('localhost')  # XXX host OK?
#		reassemblySPU = crutils.NewSPU('render')
#		mothership.AddNode(reassemblyNode)

	mothership.LayoutNodes()
	return 1


def Write_Lightning2(mothership, file):
	"""Write a Lightning-2 config to the given file handle."""
	assert Is_Lightning2(mothership)
	assert mothership.GetTemplateType() == "Lightning-2"

	print "Writing Lightning-2 config"

	template = mothership.Template
	template.UpdateFromMothership(mothership)
	template.LayoutTiles() # just in case this wasn't done already

	clientNode = FindClientNode(mothership)
	serverNode = FindServerNode(mothership)

	file.write('TEMPLATE = "Lightning-2"\n')
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

	# write tilesort SPU options
	tilesortSPU = FindTilesortSPU(mothership)
	configio.WriteSPUOptions(tilesortSPU, "TILESORT", file)

	# write render SPU options
	readbackSPU = FindReadbackSPU(mothership)
	configio.WriteSPUOptions(readbackSPU, "READBACK", file)

	# write reassemble SPU options
	reassemblySPU = FindReassemblySPU(mothership)
	configio.WriteSPUOptions(reassemblySPU, "REASSEMBLE", file)


	# write server and global options
	configio.WriteServerOptions(serverNode, file)
	configio.WriteGlobalOptions(mothership, file)

	file.write("# end of options, the rest is boilerplate\n")

	# The tiles will be recomputed when we reload the config file so
	# we can put this info after the parameter sectino.
	file.write("\n")
	file.write("TILES = %s\n" % str(template.ServerTiles))

	file.write(__ConfigBody)
	return 1

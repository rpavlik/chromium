# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

""" lightning_template.py
    Template for setting up Lightning-2 configurations.
"""

import string, cPickle, os.path, re
from wxPython.wx import *
import crutils, intdialog, hostdialog


class LightningParameters:
	"""C-style struct describing a lightning-2 configuration"""
	# This is where we set all the default parameters.
	def __init__(self, rows=1, cols=2):
		assert rows >= 1
		assert cols >= 1
		self.NumServers = 4
		self.Columns = cols
		self.Rows = rows
		self.TileWidth = 1024
		self.TileHeight = 1024
		self.Layout = 0

	def Clone(self):
		"""Return a clone of this object."""
		# We're not using the copy.copy() function since it's flakey
		p = LightningParameters()
		p.NumServers = self.NumServers
		p.Columns = self.Columns
		p.Rows = self.Rows
		p.TileWidth = self.TileWidth
		p.TileHeight = self.TileHeight
		p.Layout = 0
		return p


# Predefined tile sizes shown in the wxChoice widget (feel free to change)
CommonTileSizes = [ [64, 64],
					[128, 128],
					[256, 256],
					[512, 512] ]

# Size of the drawing page, in pixels.
PAGE_WIDTH  = 1000
PAGE_HEIGHT = 1000

BackgroundColor = wxColor(90, 150, 190)

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

# This is the guts of the tilesort configuration script.
# It's simply appended to the file after we write all the configuration options
__ConfigBody = """
import string, sys
sys.path.append( "../server" )
from mothership import *
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

# This is the guts of the configuration script.
# It's simply appended to the file after we write all the configuration options
ConfigBody = """
import string
import sys
sys.path.append( "../server" )
sys.path.append( "../tools" )
from mothership import *
from crutils import *

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
if string.find(HOSTNAME, '#') == -1:
	singleServer = 1
else:
	singleServer = 0

localHostname = os.uname()[1]

cr = CR()
cr.MTU( GLOBAL_MTU )

tilesortspu = SPU('tilesort')
tilesortspu.Conf('broadcast', TILESORT_broadcast)
tilesortspu.Conf('optimize_bucket', TILESORT_optimize_bucket)
tilesortspu.Conf('sync_on_swap', TILESORT_sync_on_swap)
tilesortspu.Conf('sync_on_finish', TILESORT_sync_on_finish)
tilesortspu.Conf('draw_bbox', TILESORT_draw_bbox)
tilesortspu.Conf('bbox_line_width', TILESORT_bbox_line_width)
#tilesortspu.Conf('fake_window_dims', fixme)
tilesortspu.Conf('scale_to_mural_size', TILESORT_scale_to_mural_size)


clientnode = CRApplicationNode()
clientnode.AddSPU(tilesortspu)

clientnode.StartDir( crbindir )
clientnode.SetApplication( os.path.join(crbindir, program) )
if GLOBAL_auto_start:
	clientnode.AutoStart( ["/bin/sh", "-c",
		"LD_LIBRARY_PATH=%s /usr/local/bin/crappfaker" % crlibdir] )


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
						   int(1.1 * col * TILE_WIDTH),
						   int(1.1 * row * TILE_HEIGHT),
						   TILE_WIDTH, TILE_HEIGHT)
			host = HOSTNAME
		else:
			renderspu.Conf('window_geometry', 0, 0, TILE_WIDTH, TILE_HEIGHT)
			host = MakeHostname(HOSTNAME, HOSTSTART + index)
		servernode = CRNetworkNode(host)

		servernode.AddTile(col * TILE_WIDTH,
						   (TILE_ROWS - row - 1) * TILE_HEIGHT,
						   TILE_WIDTH, TILE_HEIGHT)

		servernode.AddSPU(renderspu)
		servernode.Conf('optimize_bucket', SERVER_optimize_bucket)

		cr.AddNode(servernode)
		tilesortspu.AddServer(servernode, protocol='tcpip', port = 7000 + index)

		if GLOBAL_auto_start:
			servernode.AutoStart( ["/usr/bin/rsh", host,
									"/bin/sh -c 'DISPLAY=:0.0  CRMOTHERSHIP=%s  LD_LIBRARY_PATH=%s  crserver'" % (localHostname, crlibdir) ] )


cr.AddNode(clientnode)
cr.SetParam('minimum_window_size', GLOBAL_minimum_window_size)
cr.SetParam('match_window_title', GLOBAL_match_window_title)
cr.SetParam('show_cursor', GLOBAL_show_cursor)
cr.Go()

"""


#----------------------------------------------------------------------------

class LightningDialog(wxDialog):
	"""Lightning-2 configuration editor."""

	def __init__(self, parent=NULL, id=-1):
		"""Construct a Lightning-2 dialog."""
		wxDialog.__init__(self, parent, id, title="Lighting-2 Configuration",
						  style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)

		# Widget IDs
		id_MuralWidth  = 5000
		id_MuralHeight = 5001
		id_TileChoice  = 5002
		id_TileWidth   = 5003
		id_TileHeight  = 5004
		id_Layout      = 5005
		id_HostText    = 5007
		id_HostStart   = 5008
		id_HostCount   = 5009
		id_OK          = 5010
		id_Cancel      = 5011
		id_NumServers  = 5012
		id_Hostnames   = 5013

		self.HostNamePattern = "host##"
		self.HostNameStart = 0
		self.HostNameCount = 4
		self.Tiles = []

		# this sizer holds all the tilesort control widgets
		toolSizer = wxBoxSizer(wxVERTICAL)

		# Server hosts
		box = wxStaticBox(parent=self, id=-1, label="Rendering Nodes",
						  style=wxDOUBLE_BORDER)
		boxSizer = wxStaticBoxSizer(box, wxVERTICAL)
		rowSizer = wxBoxSizer(wxHORIZONTAL)
		numberLabel = wxStaticText(parent=self, id=-1, label="Number:")
		self.numberControl = wxSpinCtrl(parent=self, id=id_NumServers,
										value="1", min=1, max=10000,
										size=wxSize(70,25))
		EVT_SPINCTRL(self.numberControl, id_NumServers,
					 self.__OnNumServersChange)
		rowSizer.Add(numberLabel, flag=wxALIGN_CENTER_VERTICAL|wxALL, border=4)
		rowSizer.Add(self.numberControl,
					 flag=wxALIGN_CENTER_VERTICAL|wxALL, border=2)
		boxSizer.Add(rowSizer, flag=wxALL, border = 0)
		self.hostsButton = wxButton(parent=self, id=id_Hostnames,
									label=" Host Names... ")
		boxSizer.Add(self.hostsButton, flag=wxALIGN_CENTER|wxALL, border=4)
		EVT_BUTTON(self.hostsButton, id_Hostnames, self.__OnHostnames)
		toolSizer.Add(boxSizer, flag=wxALL|wxGROW, border=0)

		# Mural width/height (in tiles)
		box = wxStaticBox(parent=self, id=-1, label="Mural Size",
						  style=wxDOUBLE_BORDER)
		muralSizer = wxStaticBoxSizer(box, wxVERTICAL)
		flexSizer = wxFlexGridSizer(rows=2, cols=2, hgap=4, vgap=4)
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
		EVT_SPINCTRL(self.columnsControl, id_MuralWidth, self.onSizeChange)
		EVT_SPINCTRL(self.rowsControl, id_MuralHeight, self.onSizeChange)
		flexSizer.Add(columnsLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.columnsControl)
		flexSizer.Add(rowsLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.rowsControl)
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
										   value="256", min=128, max=2048,
										   size=wxSize(80,25))
		self.tileHeightLabel = wxStaticText(parent=self, id=-1,
											label="Height:")
		self.tileHeightControl = wxSpinCtrl(parent=self,
											id=id_TileHeight,
											value="256", min=128, max=2048,
											size=wxSize(80,25))
		EVT_SPINCTRL(self.tileWidthControl, id_TileWidth, self.onSizeChange)
		EVT_SPINCTRL(self.tileHeightControl, id_TileHeight, self.onSizeChange)
		EVT_CHOICE(self.tileChoice, id_TileChoice, self.onTileChoice)
		flexSizer.Add(self.tileWidthLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.tileWidthControl)
		flexSizer.Add(self.tileHeightLabel, flag=wxALIGN_CENTER_VERTICAL)
		flexSizer.Add(self.tileHeightControl)
		tileSizer.Add(self.tileChoice, flag=wxALIGN_CENTER|wxALL, border=4)
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

		# Tile layout
		layoutChoices = [ "Raster order", "Zig-zag Raster order",
						  "Spiral from center" ]
		self.layoutRadio = wxRadioBox(parent=self, id=id_Layout,
									  label="Tile Layout",
									  choices=layoutChoices,
									  majorDimension=1,
									  style=wxRA_SPECIFY_COLS )
		toolSizer.Add(self.layoutRadio, flag=wxEXPAND)
		EVT_RADIOBOX(self.layoutRadio, id_Layout, self.onLayoutChange)

		# Setup the drawing area
		self.drawArea = wxPanel(self, id=-1, style=wxSUNKEN_BORDER)
		self.drawArea.SetBackgroundColour(BackgroundColor)
		EVT_PAINT(self.drawArea, self.onPaintEvent)

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
		self.SetSizeHints(minW=400, minH=minSize[1])
		self.SetSize(minSize)

		self.__RecomputeTotalSize()
		#self.LayoutTiles()

		# Hostname dialog
		self.hostsDialog = hostdialog.HostDialog(parent=NULL, id=-1,
						title="Chromium Hosts",
						message="Specify host names for the render nodes")
		self.hostsDialog.Centre()

	# end of __init__()

	def __RecomputeTotalSize(self):
		"""Recompute the total mural size in pixels and update the widgets."""
		tileW = self.tileWidthControl.GetValue()
		tileH = self.tileHeightControl.GetValue()
		totalW = self.columnsControl.GetValue() * tileW
		totalH = self.rowsControl.GetValue() * tileH
		self.totalSizeLabel.SetLabel(str("%d x %d" % (totalW, totalH)))
		custom = 1
		for i in range(0, len(CommonTileSizes)):
			if tileW == CommonTileSizes[i][0] and tileH == CommonTileSizes[i][1]:
				self.tileChoice.SetSelection(i)
				return
		# must be custom size
		self.tileChoice.SetSelection(len(CommonTileSizes))  # "Custom"

	def __UpdateWidgetsFromVars(self):
		"""Update the widgets from internal vars."""
		self.numberControl.SetValue(self.__Mothership.Template.NumServers)
		self.layoutRadio.SetSelection(self.__Mothership.Template.Layout)
		self.columnsControl.SetValue(self.__Mothership.Template.Columns)
		self.rowsControl.SetValue(self.__Mothership.Template.Rows)

	def __UpdateVarsFromWidgets(self):
		serverNode = FindServerNode(self.__Mothership)
		serverNode.SetCount(self.numberControl.GetValue())
		#
		self.__Mothership.Template.NumServers = self.numberControl.GetValue()
		self.__Mothership.Template.Layout = self.layoutRadio.GetSelection()
		self.__Mothership.Template.Rows = self.rowsControl.GetValue()
		self.__Mothership.Template.Columns = self.columnsControl.GetValue()

	def LayoutTiles(self):
		"""Compute location and host number for the tiles."""
		cols = self.columnsControl.GetValue()
		rows = self.rowsControl.GetValue()
		numServers = self.numberControl.GetValue()
		layoutOrder = self.layoutRadio.GetSelection()
		self.Tiles = []  # list of (row, tile, server) tuples
		if layoutOrder == 0:
			# Simple raster order layout
			for i in range(rows):
				for j in range(cols):
					server = (i * cols + j) % numServers
					self.Tiles.append((i, j, server))
		elif layoutOrder == 1:
			# Slightly different raster order layout
			for i in range(rows):
				for j in range(cols):
					if i % 2 == 1:
						# odd row
						server = (i * cols + (cols - j - 1)) % numServers
					else:
						# even row
						server = (i * cols + j) % numServers
					self.Tiles.append((i, j, server))
		else:
			# Spiral outward from the center (this is a little tricky)
			assert layoutOrder == 2
			curRow = (rows - 1) / 2
			curCol = (cols - 1) / 2
			radius = 0
			march = 0
			colStep = 0
			rowStep = -1
			serv = 0
			while 1:
				assert ((rowStep == 0 and colStep != 0) or
						(rowStep != 0 and colStep == 0))
				if (curRow >= 0 and curRow < rows and
					curCol >= 0 and	curCol < cols):
					# save this tile location
					#server = len(self.Tiles) % numServers
					server = serv % numServers
					assert (curRow, curCol, server) not in self.Tiles
					self.Tiles.append((curRow, curCol, server))
					# check if we're done
					if len(self.Tiles) >= rows * cols:
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

		
	#----------------------------------------------------------------------
	# Widget callback functions

	def __OnNumServersChange(self, event):
		"Called when number of servers changes"""
		self.__Mothership.Template.NumServers = self.numberControl.GetValue()
		self.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	def __OnHostnames(self,event):
		"""Called when the hostnames button is pressed."""
		sortlast = self.__Mothership.Sortlast
		clientNode = FindClientNode(self.__Mothership)
		self.hostsDialog.SetHostPattern(clientNode.GetHostNamePattern())
		self.hostsDialog.SetCount(clientNode.GetCount())
		self.hostsDialog.SetHosts(clientNode.GetHosts())
		if self.hostsDialog.ShowModal() == wxID_OK:
			clientNode.SetHostNamePattern(self.hostsDialog.GetHostPattern())
			clientNode.SetHosts(self.hostsDialog.GetHosts())
		#self.drawArea.Refresh()
		#self.dirty = true

	def onSizeChange(self, event):
		"""Called when tile size changes with spin controls."""
		self.__RecomputeTotalSize()
		self.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	def onLayoutChange(self, event):
		"""Called when Layout order changes."""
		self.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	def onTileChoice(self, event):
		"""Called when tile size changes with combo-box control."""
		i = self.tileChoice.GetSelection()
		if i < len(CommonTileSizes):
			w = CommonTileSizes[i][0]
			h = CommonTileSizes[i][1]
			self.tileWidthControl.SetValue(w)
			self.tileHeightControl.SetValue(h)
		self.__RecomputeTotalSize()
		self.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	# Called when hostname or first host index changes
	def onHostChange(self, event):
		"""Called when the host name pattern or first index changes."""
		self.HostNamePattern = self.hostText.GetValue()
		self.HostNameStart = self.hostStart.GetValue()
		self.HostNameCount = self.hostCount.GetValue()
		self.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	def _onOK(self, event):
		"""Called by OK button"""
		self.__UpdateVarsFromWidgets()
		self.EndModal(wxID_OK)

	def _onCancel(self, event):
		"""Called by Cancel button"""
		self.EndModal(wxID_CANCEL)

	def onPaintEvent(self, event):
		""" Respond to a request to redraw the contents of our drawing panel.
		"""
		dc = wxPaintDC(self.drawArea)
#		self.drawArea.PrepareDC(dc)  # only for scrolled windows
		dc.BeginDrawing()
		dc.SetPen(wxBLACK_PEN);
		dc.SetBrush(wxLIGHT_GREY_BRUSH);

		# border around the window and space between the tiles
		border = 20
		space = 2
		
		# Get current settings
		size = self.drawArea.GetSize()
		cols = self.columnsControl.GetValue()
		rows = self.rowsControl.GetValue()
		tileWidth = self.tileWidthControl.GetValue()
		tileHeight = self.tileHeightControl.GetValue()
		layout = self.layoutRadio.GetSelection()

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
		hosts = FindServerNode(self.__Mothership).GetHosts()
		numColors = len(ServerColors)
		numServers = self.numberControl.GetValue()
		for (row, col, server) in self.Tiles:
			x = col * (w + space) + border
			y = row * (h + space) + border
			color = server % numColors
			dc.SetBrush(wxBrush(ServerColors[color]))
			dc.DrawRectangle(x, y, w,h)
			if server < len(hosts):
				s = hosts[server]
			else:
				s = hosts[-1]
			#crutils.MakeHostname(self.HostNamePattern, self.HostNameStart + server)
			(tw, th) = dc.GetTextExtent(s)
			dx = (w - tw) / 2
			dy = (h - th) / 2
			dc.DrawText(s, x+dx, y+dy)
		dc.EndDrawing()

		#for i in range(rows):
		#	for j in range(cols):
		#		x = j * (w + space) + border
		#		y = i * (h + space) + border
		#		server = (i * cols + j) % numServers
		#		color = server % numColors
		#		dc.SetBrush(wxBrush(ServerColors[color]))
		#		dc.DrawRectangle(x, y, w,h)
		#		s = MakeHostname(self.HostNamePattern, self.HostNameStart + server)
		#		(tw, th) = dc.GetTextExtent(s)
		#		dx = (w - tw) / 2
		#		dy = (h - th) / 2
		#		dc.DrawText(s, x+dx, y+dy)
		dc.EndDrawing()

		# draw top width label
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

	def SetMothership(self, mothership):
		"""Specify the mothership to modify.
		mothership is a Mothership object.
		"""
		self.__Mothership = mothership
		# update all the widgets to the template's values
		self.__UpdateWidgetsFromVars()
		self.__RecomputeTotalSize()
		self.LayoutTiles()


def Create_Lightning2(parentWindow, mothership):
	"""Create a tilesort configuration"""
	defaultMuralSize = crutils.GetSiteDefault("mural_size")
	if defaultMuralSize:
		dialogDefaults = [ 1, defaultMuralSize[0], defaultMuralSize[1] ]
	else:
		dialogDefaults = [1, 2, 1]

	# XXX also prompt for tile size?
	dialog = intdialog.IntDialog(parent=parentWindow, id=-1,
								 title="Lightning-2 Template",
								 labels=["Number of application nodes:",
										 "Number of server nodes:",
										 "Number of Columns:",
										 "Number of Rows:"],
								 defaultValues=[1, 4, 2, 2], maxValue=10000)
	dialog.Centre()
	if dialog.ShowModal() == wxID_CANCEL:
		dialog.Destroy()
		return 0

	# Init tilesort parameters
	values = dialog.GetValues()
	numClients = values[0]
	numServers = values[1]
	cols = values[2]
	rows = values[3]
	mothership.Template = LightningParameters(rows, cols)

	# build the graph
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


def Is_Lightning2(mothership):
	"""Test if the mothership describes a lightning-2 configuration.
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


def Edit_Lightning2(parentWindow, mothership):
	"""Edit parameters for a Lightning2 template"""
	# XXX we only need to create one instance of the Lightning2Frame() and
	# reuse it in the future.
	t = Is_Lightning2(mothership)
	if t:
		clientNode = FindClientNode(mothership)
		serverNode = FindServerNode(mothership)
		print "Edit lightning-2"
	else:
		print "This is not a tilesort configuration!"
		return

	d = LightningDialog(parent=parentWindow)
	d.Centre()
	backupLightningParams = mothership.Template.Clone()
	d.SetMothership(mothership)

	if d.ShowModal() == wxID_CANCEL:
		# restore original values
		mothership.Lightning = backupLightningParams
	else:
		# update mothership with new values
		# already done in __UpdateVarsFromWidgets()
		pass



def Read_Lightning2(mothership, fileHandle):
	"""Read a tilesort config from the given file handle."""
	pass


def Write_Lightning2(mothership, file):
	"""Write a tilesort config to the given file handle."""
	assert Is_Lightning2(mothership)
	assert mothership.GetTemplateType() == "Lightning2"

	print "Writing tilesort config"


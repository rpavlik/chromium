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


# Known issues:
# 1. All tiles must be the same size
# 2. Tiles can't overlap
# 3. Need to support arbitrary SPUs on client/server nodes.


import string, cPickle, os.path, re, sys
from wxPython.wx import *
import traceback, types
import intdialog, spudialog, hostdialog, textdialog
import crutils, crtypes, configio
import templatebase
sys.path.append("../server")
import crconfig


class TilesortParameters:
	"""This class describes the parameters of a tilesort configuration."""
	def __init__(self, rows=1, cols=2):
		assert rows >= 1
		assert cols >= 1
		self.Columns = cols
		self.Rows = rows
		self.TileWidth = 1024
		self.TileHeight = 1024
		self.RightToLeft = 0
		self.BottomToTop = 0
		self.ServerHosts = ["localhost"]
		self.ServerPattern = ("localhost", 1)
		self.Tiles = []
		self.ServerTiles = [ [] ]
		
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
		p.ServerHosts = self.ServerHosts[:]
		p.ServerPattern = self.ServerPattern
		p.Tiles = self.Tiles[:]
		p.ServerTiles = self.ServerTiles[:]
		return p

	def UpdateFromMothership(self, mothership):
		serverNode = FindServerNode(mothership)
		clientNode = FindClientNode(mothership)
		#self.Columns = ??
		#self.Rows = ??
		#self.TileWidth = ??
		#self.TileHeight = ??
		#self.RightToLeft = ??
		#self.BottomToTop = ??
		self.ServerHosts = serverNode.GetHosts()[:] # [:] makes a copy
		self.ServerPattern = serverNode.GetHostNamePattern()

	def LayoutTiles(self):
		"""Compute locations and hosts for the tiles."""
		self.Tiles = []  # tuples (row, col, server) for drawing
		self.ServerTiles = [ [] ] # array [server] of array of (x,y,w,h)
		for i in range(self.Rows):
			for j in range(self.Columns):
				if self.BottomToTop == 0:
					row = i
				else:
					row = self.Rows - i - 1
				if self.RightToLeft == 0:
					col = j
				else:
					col = self.Columns - j - 1
				# compute mural tile geometry
				mx = col * self.TileWidth
				my = row * self.TileHeight
				muralTile = (mx, my, self.TileWidth, self.TileHeight)
				# compute server index
				server = row * self.Columns + col
				if server >= len(self.ServerHosts):
					server = len(self.ServerHosts) - 1
				# save tile
				self.Tiles.append( (row, col, server) )
				# save per-server mural tile
				while len(self.ServerTiles) - 1 < server:
					self.ServerTiles.append( [] )
				self.ServerTiles[server].append( muralTile )
		return


# Predefined tile sizes shown in the wxChoice widget (feel free to change)
CommonTileSizes = [ [128, 128],
					[256, 256],
					[512, 512],
					[1024, 1024],
					[1280, 1024],
					[1600, 1200] ]

BackgroundColor = wxColor(70, 170, 130)


#----------------------------------------------------------------------------

_ImportsSection = """
import string, sys, getopt
sys.path.append( "../server" )
from mothership import *

"""

# This is the guts of the tilesort configuration script.
# It's simply appended to the file after we write all the configuration options
_ConfigBody = """
def Usage():
	print "Usage:"
	print "  %s [--help] [-c columns] [-r rows] [-w tileWidth]" % sys.argv[0]
	print "     [-h tileHeight] [-s servers] [program]"
	sys.exit(0)
	

# Init globals
PROGRAM = ""
ZEROTH_ARG = ""
AUTO_START = 0

# Look for some special app and mothership params
for (name, value) in APP_OPTIONS:
	if name == "application":
		PROGRAM = value
	elif name == "zeroth_arg":
		ZEROTH_ARG = value
for (name, value) in MOTHERSHIP_OPTIONS:
	if name == "auto_start":
		AUTO_START = value

# Check for program name/args on command line
try:
	(opts, args) = getopt.getopt(sys.argv[1:], "c:r:w:h:s:", ["help"])
except getopt.GetoptError:
	Usage()

for (name, value) in opts:
	if name == "--help":
		Usage()
	elif name == "-c":
		TILE_COLS = int(value)
	elif name == "-r":
		TILE_ROWS = int(value)
	elif name == "-w":
		TILE_WIDTH = int(value)
	elif name == "-h":
		TILE_HEIGHT = int(value)
	elif name == "-s":
		SERVER_HOSTS = str.split(value, ",")
		
if len(args) > 0:
	PROGRAM = args[0]


print "--- Tilesort Template ---"
print "Mural size: %d cols x %d rows" % (TILE_COLS, TILE_ROWS)
print "Tile size: %d x %d" % (TILE_WIDTH, TILE_HEIGHT)
print "Total size: %d x %d" % (TILE_WIDTH * TILE_COLS, TILE_HEIGHT * TILE_ROWS)
print "Servers: %s" % SERVER_HOSTS
print "Program: %s" % PROGRAM
print "-------------------------"


# Determine if tiles are on one server or many
singleServer = 1
for i in range(1, len(SERVER_HOSTS)):
	if SERVER_HOSTS[i] != SERVER_HOSTS[0]:
		singleServer = 0
		break

localHostname = os.uname()[1]


cr = CR()


tilesortSPUs = []
appNodes = []

for i in range(NUM_APP_NODES):
	tilesortspu = SPU('tilesort')
	for (name, value) in TILESORT_OPTIONS:
		tilesortspu.Conf(name, value)
	tilesortSPUs.append(tilesortspu)

	appnode = CRApplicationNode(APP_NODES[i])
	for (name, value) in APP_OPTIONS:
		appnode.Conf(name, value)

	appnode.AddSPU(tilesortspu)

	# argument substitutions
	if PROGRAM != "":
		if i == 0 and ZEROTH_ARG != "":
			app_string = string.replace( PROGRAM, '%0', ZEROTH_ARG)
		else:
			app_string = string.replace( PROGRAM, '%0', '' )
		app_string = string.replace( app_string, '%I', str(i) )
		app_string = string.replace( app_string, '%N', str(NUM_APP_NODES) )
		appnode.Conf('application', app_string )

		if AUTO_START:
			appnode.AutoStart( ["/bin/sh", "-c",
				"LD_LIBRARY_PATH=%s /usr/local/bin/crappfaker" % crlibdir] )

	appNodes.append(appnode)


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
		for (name, value) in RENDER_OPTIONS:
			renderspu.Conf(name, value)

		if singleServer:
			renderspu.Conf('window_geometry', [
						   int(1.1 * j * TILE_WIDTH),
						   int(1.1 * i * TILE_HEIGHT),
						   TILE_WIDTH, TILE_HEIGHT ] )
			host = SERVER_HOSTS[0]
		else:
			renderspu.Conf('window_geometry', [0, 0, TILE_WIDTH, TILE_HEIGHT])
			host = SERVER_HOSTS[index]
		servernode = CRNetworkNode(host)

		servernode.AddTile(col * TILE_WIDTH,
						   (TILE_ROWS - row - 1) * TILE_HEIGHT,
						   TILE_WIDTH, TILE_HEIGHT)

		servernode.AddSPU(renderspu)
		for (name, value) in SERVER_OPTIONS:
			servernode.Conf(name, value)

		cr.AddNode(servernode)
		for i in range(NUM_APP_NODES):
			tilesortSPUs[i].AddServer(servernode, protocol='tcpip', port = 7000 + index)

		if AUTO_START:
			servernode.AutoStart( ["/usr/bin/rsh", host,
									"/bin/sh -c 'DISPLAY=:0.0  CRMOTHERSHIP=%s  LD_LIBRARY_PATH=%s  crserver'" % (localHostname, crlibdir) ] )


# Add nodes to mothership
for i in range(NUM_APP_NODES):
	cr.AddNode(appNodes[i])

# Set mothership params
for (name, value) in MOTHERSHIP_OPTIONS:
	cr.Conf(name, value)

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
		""" Construct a TilesortDialog."""
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
		id_HELP        = 3013

		# init misc member vars
		self.__Mothership = 0  # only need this to edit tilesort SPU options
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
		self.columnsControl = wxSpinCtrl(parent=self, id=id_MuralWidth,
									   value="1", min=1, max=16,
									   size=wxSize(50,25))
		rowsLabel = wxStaticText(parent=self, id=-1, label="Rows:")
		self.rowsControl = wxSpinCtrl(parent=self,
										id=id_MuralHeight,
										value="1", min=1, max=16,
										size=wxSize(50,25))
		EVT_SPINCTRL(self.columnsControl, id_MuralWidth, self.__OnSizeChange)
		EVT_SPINCTRL(self.rowsControl, id_MuralHeight, self.__OnSizeChange)
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
		self.hostsDialog.Centre()

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

		self.__UpdateDependentWidgets()
	# end of __init__()

	def __UpdateDependentWidgets(self):
		"""Called whenever the mural width/height or tile width/height changes.
		Recompute the total mural size in pixels and update the widgets."""
		tileW = self.tileWidthControl.GetValue()
		tileH = self.tileHeightControl.GetValue()
		totalW = self.columnsControl.GetValue() * tileW
		totalH = self.rowsControl.GetValue() * tileH
		self.totalSizeLabel.SetLabel(str("%d x %d" % (totalW, totalH)))
		for i in range(0, len(CommonTileSizes)):
			if (tileW == CommonTileSizes[i][0] and
				tileH == CommonTileSizes[i][1]):
				self.tileChoice.SetSelection(i)
				return
		# must be custom size
		self.tileChoice.SetSelection(len(CommonTileSizes))  # "Custom"

	def __UpdateVarsFromWidgets(self):
		"""Get current widget values and update the tilesort parameters."""
		self.Template.Columns = self.columnsControl.GetValue()
		self.Template.Rows = self.rowsControl.GetValue()
		self.Template.TileWidth = self.tileWidthControl.GetValue()
		self.Template.TileHeight = self.tileHeightControl.GetValue()
		self.Template.RightToLeft = self.hLayoutRadio.GetSelection()
		self.Template.BottomToTop = self.vLayoutRadio.GetSelection()
		# XXX set render SPU's window_geometry = tile size!!

	def __UpdateWidgetsFromVars(self):
		"""Set widget values to the tilesort parameters."""
		self.columnsControl.SetValue(self.Template.Columns)
		self.rowsControl.SetValue(self.Template.Rows)
		self.tileWidthControl.SetValue(self.Template.TileWidth)
		self.tileHeightControl.SetValue(self.Template.TileHeight)
		self.hLayoutRadio.SetSelection(self.Template.RightToLeft)
		self.vLayoutRadio.SetSelection(self.Template.BottomToTop)

	# ----------------------------------------------------------------------
	# Event handling

	def __OnSizeChange(self, event):
		"""Called when tile size changes with spin controls."""
		self.__UpdateVarsFromWidgets()
		self.__UpdateDependentWidgets()
		self.Template.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	def __OnLayoutChange(self, event):
		"""Called when left/right top/bottom layout changes."""
		self.__UpdateVarsFromWidgets()
		self.__UpdateDependentWidgets()
		self.Template.LayoutTiles()
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
		self.__UpdateDependentWidgets()
		self.Template.LayoutTiles()
		self.drawArea.Refresh()
		self.dirty = true

	def __OnHostnames(self, event):
		"""Called when the hostnames button is pressed."""
		self.hostsDialog.SetHostPattern(self.Template.ServerPattern)
		self.hostsDialog.SetHosts(self.Template.ServerHosts)
		self.hostsDialog.SetCount(self.Template.Rows * self.Template.Columns)
		if self.hostsDialog.ShowModal() == wxID_OK:
			self.Template.ServerHosts = self.hostsDialog.GetHosts()
			self.Template.ServerPattern = self.hostsDialog.GetHostPattern()
			self.Template.LayoutTiles()
			self.drawArea.Refresh()

	def __OnTilesortOptions(self, event):
		"""Called when Tilesort Options button is pressed."""
		tilesortSPU = FindTilesortSPU(self.__Mothership)
		# create the dialog
		dialog = spudialog.SPUDialog(parent=self, id=-1,
									 title="Tilesort SPU Options",
									 optionList = tilesortSPU.GetOptions())
		dialog.Centre()
		# wait for OK or cancel
		if dialog.ShowModal() == wxID_OK:
			# save the new values/options
			for opt in tilesortSPU.GetOptions():
				value = dialog.GetValue(opt.Name)
				tilesortSPU.SetOption(opt.Name, value)
		return

	def _onOK(self, event):
		"""Called by OK button"""
		self.EndModal(wxID_OK)

	def _onCancel(self, event):
		"""Called by Cancel button"""
		self.EndModal(wxID_CANCEL)

	def _onHelp(self, event):
		"""Called by Help button"""
		d = textdialog.TextDialog(parent=self, id = -1,
								  title="Tile-sort Help")
		d.LoadPage("../../doc/tilesort_template.html")
		# Hmmm, I want to call d.Show() so this isn't modal/blocking but
		# that doesn't seem to work.
		d.ShowModal();
		d.Destroy()

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
		cols = self.columnsControl.GetValue()
		rows = self.rowsControl.GetValue()
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

		# compute tile size (in pixels)
		w = tileWidth * scale
		h = tileHeight * scale

		# draw the tiles as boxes
		for (row, col, server) in self.Template.Tiles:
			x = col * (w + space) + border
			y = row * (h + space) + border
			dc.DrawRectangle(x, y, w, h)
			s = self.Template.ServerHosts[server]
			dc.DrawText(s, x+3, y+3)
		dc.EndDrawing()

	def ShowModal(self, mothership):
		"""Show the dialog and block until OK or Cancel is chosen."""
		# Load the template values
		self.__Mothership = mothership
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
			serverNode.SetCount(mothership.Template.Rows *
								mothership.Template.Columns)
			serverNode.SetHostNamePattern(mothership.Template.ServerPattern)
			serverNode.SetHosts(mothership.Template.ServerHosts)
			i = 0
			for tileList in mothership.Template.ServerTiles:
				serverNode.SetTiles(tileList, i)
				i += 1
		return retVal



#----------------------------------------------------------------------

class TilesortTemplate(templatebase.TemplateBase):
	"""Template for creating/editing/reading/writing tilesort configs."""
	def __init__(self):
		# no-op for now
		pass

	def Name(self):
		return "Tilesort"

	def Create(self, parentWindow, mothership):
		"""Create the nodes/spus/etc for a tilesort config."""
		defaultMuralSize = crutils.GetSiteDefault("mural_size")
		if defaultMuralSize:
			dialogDefaults = [ 1, defaultMuralSize[0], defaultMuralSize[1] ]
		else:
			dialogDefaults = [1, 2, 1]

		dialog = intdialog.IntDialog(parent=parentWindow, id=-1,
									 title="Tilesort Template",
									 labels=["Number of application nodes:",
											 "Mural Columns:",
											 "Mural Rows:"],
									 defaultValues=dialogDefaults,
									 minValue=1, maxValue=10000)
		dialog.Centre()
		if dialog.ShowModal() == wxID_CANCEL:
			dialog.Destroy()
			return 0

		# Init tilesort parameters
		values = dialog.GetValues()
		numClients = values[0]
		cols = values[1]
		rows = values[2]
		mothership.Template = TilesortParameters(rows, cols)

		defaultScreenSize = crutils.GetSiteDefault("screen_size")
		if defaultScreenSize:
			mothership.Template.TileWidth = defaultScreenSize[0]
			mothership.Template.TileHeight = defaultScreenSize[1]

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

		# Do initial tile layout
		mothership.Template.UpdateFromMothership(mothership)
		mothership.Template.LayoutTiles()

		# Set the initial tile list for each server
		i = 0
		for tileList in mothership.Template.ServerTiles:
			serverNode.SetTiles(tileList, i)
			i += 1

		# done with the dialog
		dialog.Destroy()
		return 1

	def Validate(self, mothership):
		"""Test if the mothership config is a tilesort config."""
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

	def Edit(self, parentWindow, mothership):
		"""Open editor window and edit the mothership config"""
		if not self.Validate(mothership):
			print "Editing - This is not a tilesort config!!!!"
			return 0
		dialog = TilesortDialog(parent=parentWindow)
		dialog.Centre()
		retVal = dialog.ShowModal(mothership)
		dialog.Destroy()
		return retVal

	def Read(self, mothership, fileHandle):
		"""Read tilesort config from file"""
		mothership.Template = TilesortParameters()

		serverNode = crtypes.NetworkNode()
		renderSPU = crutils.NewSPU("render")
		serverNode.AddSPU(renderSPU)

		clientNode = crtypes.ApplicationNode()
		tilesortSPU = crutils.NewSPU("tilesort")
		clientNode.AddSPU(tilesortSPU)
		tilesortSPU.AddServer(serverNode)

		mothership.AddNode(clientNode)
		mothership.AddNode(serverNode)

		numClients = 1

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
			if re.match("^import", l):
				pass  # ignore
			elif re.match("^sys.path.append", l):
				pass  # ignore
			elif re.match("from mothership import", l):
				pass  # ignore
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
			elif re.match("^BOTTOM_TO_TOP = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				mothership.Template.BottomToTop = int(l[v.start() : v.end()])
			elif re.match("^RIGHT_TO_LEFT = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				mothership.Template.RightToLeft = int(l[v.start() : v.end()])
			elif re.match("^SERVER_HOSTS = ", l):
				v = re.search(listPat + "$", l)
				hosts = eval(l[v.start() : v.end()])
				serverNode.SetHosts(hosts)
			elif re.match("^SERVER_PATTERN = ", l):
				v = re.search(tuplePat + "$", l)
				pattern = eval(l[v.start() : v.end()])
				serverNode.SetHostNamePattern(pattern)
			elif re.match("^APP_HOSTS = ", l):
				v = re.search(listPat + "$", l)
				hosts = eval(l[v.start() : v.end()])
				clientNode.SetHosts(hosts)
			elif re.match("^APP_PATTERN = ", l):
				v = re.search(tuplePat + "$", l)
				pattern = eval(l[v.start() : v.end()])
				clientNode.SetHostNamePattern(pattern)
			elif re.match("^NUM_APP_NODES = " + integerPat + "$", l):
				v = re.search(integerPat, l)
				numClients = int(l[v.start() : v.end()])
			elif re.match("^TILESORT_OPTIONS = \[", l):
				tilesortSPU.GetOptions().Read(fileHandle)
			elif re.match("^RENDER_OPTIONS = \[", l):
				renderSPU.GetOptions().Read(fileHandle)
			elif re.match("^SERVER_OPTIONS = \[", l):
				serverNode.GetOptions().Read(fileHandle)
			elif re.match("^APP_OPTIONS = \[", l):
				# replace found occurances of 'crbindir' with the real directory
				substitutions = [ ("crbindir", "'%s'" % crconfig.crbindir) ]
				clientNode.GetOptions().Read(fileHandle, substitutions)
			elif re.match("^MOTHERSHIP_OPTIONS = \[", l):
				mothership.GetOptions().Read(fileHandle)
			elif re.match("^# end of options", l):
				# that's the end of the variables
				break
			elif (l != "") and (not re.match("\s*#", l)):
				print "unrecognized line: %s" % l
		# endwhile

		clientNode.SetCount(numClients)
		serverNode.SetCount(mothership.Template.Rows * mothership.Template.Columns)
		mothership.LayoutNodes()
		return 1

	def Write(self, mothership, fileHandle):
		"""Write tilesort config to file."""
		if not self.Validate(mothership):
			print "Writing - This is not a tilesort config!!!!"
			return 0

		template = mothership.Template
		clientNode = FindClientNode(mothership)
		serverNode = FindServerNode(mothership)

		file = fileHandle
		file.write('TEMPLATE = "%s"\n' % self.Name())
		file.write(_ImportsSection)
		file.write("TILE_ROWS = %d\n" % template.Rows)
		file.write("TILE_COLS = %d\n" % template.Columns)
		file.write("TILE_WIDTH = %d\n" % template.TileWidth)
		file.write("TILE_HEIGHT = %d\n" % template.TileHeight)
		file.write("RIGHT_TO_LEFT = %d\n" % template.RightToLeft)
		file.write("BOTTOM_TO_TOP = %d\n" % template.BottomToTop)
		file.write("SERVER_HOSTS = %s\n" % str(serverNode.GetHosts()))
		file.write('SERVER_PATTERN = %s\n' % str(serverNode.GetHostNamePattern()))
		file.write("APP_HOSTS = %s\n" % str(clientNode.GetHosts()))
		file.write('APP_PATTERN = %s\n' % str(clientNode.GetHostNamePattern()))
		file.write("NUM_APP_NODES = %d\n" % clientNode.GetCount())

		# write tilesort SPU options
		tilesortSPU = FindTilesortSPU(mothership)
		fakeDims = tilesortSPU.GetOption('fake_window_dims')
		if fakeDims[0] == 0 and fakeDims[1] == 0:
			# set fake window dims to mural size
			w = template.Columns * template.TileWidth
			h = template.Rows * template.TileHeight
			tilesortSPU.SetOption('fake_window_dims', [w, h])
		tilesortSPU.GetOptions().Write(file, "TILESORT_OPTIONS")

		# write render SPU options
		renderSPU = FindRenderSPU(mothership)
		renderSPU.GetOptions().Write(file, "RENDER_OPTIONS")

		# write server options
		serverNode.GetOptions().Write(file, "SERVER_OPTIONS")

		# write app node options
		substitutions = [ (crconfig.crbindir, 'crbindir') ]
		clientNode.GetOptions().Write(file, "APP_OPTIONS", substitutions)

		# write mothership options
		mothership.GetOptions().Write(file, "MOTHERSHIP_OPTIONS")

		file.write("# end of options, the rest is boilerplate (do not remove this line!)\n")
		file.write(_ConfigBody)
		return 1


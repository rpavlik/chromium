""" tilesort_tmp.py
    Tilesort template module.
"""

import string, cPickle, os.path, re
from wxPython.wx import *
import traceback, types
from spudialog import *
from crutils import *


CommonTileSizes = [ [128, 128],
					[256, 256],
					[512, 512],
					[1024, 1024],
					[1280, 1024],
					[1600, 1200] ]

BackgroundColor = wxColor(70, 170, 130)

# We use the SPU options infrastructure to handle server and global options!
ServerOptions = [
	("optimize_bucket", "Optimized Extent Bucketing", "BOOL", 1, [1], [], []),
	("lighting2", "Generate Lightning-2 Strip Headers", "BOOL", 1, [0], [], [])
]

GlobalOptions = [
	("minimum_window_size", "Minimum Chromium App Window Size (w h)", "INT", 2, [0, 0], [0, 0], []),
	("match_window_title", "Match App Window Title", "STRING", 1, [""], [], []),
	("show_cursor", "Show Virtual cursor", "BOOL", 1, [0], [], []),
	("MTU", "Mean Transmission Unit (bytes)", "INT", 1, [1024*1024], [0], []),
	("default_app", "Default Application Program", "STRING", 1, [""], [], []),
	("auto_start", "Automatically Start Servers", "BOOL", 1, [0], [], [])
]

# This is the guts of the tilesort configuration script.
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
			host = MakeHostname(HOSTNAME, FIRSTHOST + index)
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


class TilesortOptions:
	"""C-style struct describing a tilesort configuration"""
	TileRows = 0
	TileCols = 0
	TileWidth = 1024
	TileHeight = 1024
	RightToLeft = 0
	BottomToTop = 0
	Hostname = "host##"
	FirstHost = 1
	NumClients = 1



#----------------------------------------------------------------------------

class TilesortFrame(wxFrame):
	"""Tilesort configuration editor."""

	def __init__(self, parent=NULL, id=-1):
		""" Construct a TilesortFrame."""
		wxFrame.__init__(self, parent, id, title="Tilesort Configuration",
						 style = wxDEFAULT_FRAME_STYLE | wxWANTS_CHARS |
						 wxNO_FULL_REPAINT_ON_RESIZE)

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

		self.HostNamePattern = "host##"
		self.HostNameStart = 0

		self.topPanel = wxPanel(self, id=-1)
		toolSizer = wxBoxSizer(wxVERTICAL)

		# Mural width/height (in tiles)
		box = wxStaticBox(parent=self.topPanel, id=-1, label="Mural Size",
						  style=wxDOUBLE_BORDER)
		muralSizer = wxStaticBoxSizer(box, wxVERTICAL)
		flexSizer = wxFlexGridSizer(rows=2, cols=2, hgap=4, vgap=4)
		columnsLabel = wxStaticText(parent=self.topPanel, id=-1,
									label="Columns:")
		self.widthControl = wxSpinCtrl(parent=self.topPanel, id=id_MuralWidth,
									   value="4", min=1, max=16,
									   size=wxSize(50,25))
		rowsLabel = wxStaticText(parent=self.topPanel, id=-1, label="Rows:")
		self.heightControl = wxSpinCtrl(parent=self.topPanel,
										id=id_MuralHeight,
										value="3", min=1, max=16,
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
		box = wxStaticBox(parent=self.topPanel, id=-1, label="Tile Size",
						  style=wxDOUBLE_BORDER)
		tileSizer = wxStaticBoxSizer(box, wxVERTICAL)
		flexSizer = wxFlexGridSizer(rows=2, cols=2, hgap=4, vgap=4)
		tileChoices = []
		for i in CommonTileSizes:
			tileChoices.append( str("%d x %d" % (i[0], i[1])) )
		tileChoices.append("Custom")
		self.tileChoice = wxChoice(parent=self.topPanel, id=id_TileChoice,
								   choices=tileChoices)
		flexSizer = wxFlexGridSizer(rows=2, cols=2, hgap=4, vgap=4)
		self.tileWidthLabel = wxStaticText(parent=self.topPanel, id=-1,
										   label="Width:")
		self.tileWidthControl = wxSpinCtrl(parent=self.topPanel,
										   id=id_TileWidth,
										   value="512", min=128, max=2048,
										   size=wxSize(80,25))
		self.tileHeightLabel = wxStaticText(parent=self.topPanel, id=-1,
											label="Height:")
		self.tileHeightControl = wxSpinCtrl(parent=self.topPanel,
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
		box = wxStaticBox(parent=self.topPanel, id=-1, label="Total Size",
						  style=wxDOUBLE_BORDER)
		totalSizer = wxStaticBoxSizer(box, wxVERTICAL)
		self.totalSizeLabel = wxStaticText(parent=self.topPanel, id=-1,
										   label="??")
		totalSizer.Add(self.totalSizeLabel, flag=wxEXPAND)
		toolSizer.Add(totalSizer, flag=wxEXPAND)

		hChoices = [ 'Left to right', 'Right to left' ]
		self.hLayoutRadio = wxRadioBox(parent=self.topPanel, id=id_hLayout,
									   label="Horizontal Layout",
									   choices=hChoices,
									   majorDimension=1,
									   style=wxRA_SPECIFY_COLS )
		toolSizer.Add(self.hLayoutRadio, flag=wxEXPAND)
		vChoices = [ 'Top to bottom', 'Bottom to top' ]
		self.vLayoutRadio = wxRadioBox(parent=self.topPanel, id=id_vLayout,
									   label="Vertical Layout",
									   choices=vChoices,
									   majorDimension=1,
									   style=wxRA_SPECIFY_COLS )
		toolSizer.Add(self.vLayoutRadio, flag=wxEXPAND)
		EVT_RADIOBOX(self.hLayoutRadio, id_hLayout, self.__OnLayoutChange)
		EVT_RADIOBOX(self.vLayoutRadio, id_vLayout, self.__OnLayoutChange)

		# Host naming
		box = wxStaticBox(parent=self.topPanel, id=-1, label="Host Names",
						  style=wxDOUBLE_BORDER)
		hostSizer = wxStaticBoxSizer(box, wxVERTICAL)
		# XXX should probably use a wxComboBox here so we can keep a small
		# history of frequently used hostname pattern strings.
		self.hostText = wxTextCtrl(parent=self.topPanel, id=id_HostText,
								   value=self.HostNamePattern)
		EVT_TEXT(self.hostText, id_HostText, self.__OnHostChange)
		hostSizer.Add(self.hostText, flag=wxEXPAND)

		spinSizer = wxBoxSizer(wxHORIZONTAL)
		firstLabel = wxStaticText(parent=self.topPanel, id=-1,
								  label="First index: ")
		spinSizer.Add(firstLabel, flag=wxALIGN_CENTER_VERTICAL)
		self.hostSpin = wxSpinCtrl(parent=self.topPanel, id=id_HostIndex,
								   value=str(self.HostNameStart), min=0,
								   size=wxSize(60,25))
		EVT_SPINCTRL(self.hostSpin, id_HostIndex, self.__OnHostChange)
		spinSizer.Add(self.hostSpin)

		hostSizer.Add(spinSizer, border=4, flag=wxTOP)
		toolSizer.Add(hostSizer, flag=wxEXPAND)

		# SPU option buttons
		self.tilesortButton = wxButton(parent=self.topPanel, id=-1,
									   label="Tilesort SPU Options...")
		toolSizer.Add(self.tilesortButton, flag=wxALL, border=4)
		#self.renderButton = wxButton(parent=self.topPanel, id=-1,
		#							 label="Render SPU Options...")
		#toolSizer.Add(self.renderButton, flag=wxALL, border=4)

		# Setup the drawing area
		self.drawArea = wxPanel(self.topPanel, id=-1, style=wxSUNKEN_BORDER)
		self.drawArea.SetBackgroundColour(BackgroundColor)
		EVT_PAINT(self.drawArea, self.__OnPaintEvent)

		# Position everything in the window.
		topSizer = wxBoxSizer(wxHORIZONTAL)
		topSizer.Add(toolSizer, option=0,
					 flag=wxTOP | wxLEFT | wxRIGHT | wxALIGN_TOP, border=5)
		topSizer.Add(self.drawArea, 1, wxEXPAND)

		self.topPanel.SetAutoLayout(true)
		self.topPanel.SetSizer(topSizer)

		minSize = topSizer.GetMinSize()
		minSize[0] = 600
		minSize[1] += 10
		self.SetSizeHints(minW=250, minH=200)
		self.SetSize(minSize)

		self.dirty     = false

		self.__RecomputeTotalSize()
		
		# Make the Tilesort SPU options dialog
		self.tilesortInfo = GetSPUOptions("tilesort")
		assert self.tilesortInfo
		(tilesortParams, tilesortOptions) = self.tilesortInfo
		self.TilesortDialog = SPUDialog(parent=NULL, id=-1,
										title="Tilesort SPU Options",
										options=tilesortOptions)

		# Make the render SPU options dialog
		self.renderInfo = GetSPUOptions("render")
		assert self.renderInfo
		(renderParams, renderOptions) = self.renderInfo
		self.RenderDialog = SPUDialog(parent=NULL, id=-1,
									  title="Render SPU Options",
									  options=renderOptions)

		# Make the server options dialog
		self.ServerDialog = SPUDialog(parent=NULL, id=-1,
									  title="Server Options",
									  options=ServerOptions)

		# Make the global options dialog
		self.GlobalDialog = SPUDialog(parent=NULL, id=-1,
									  title="Global Chromium Options",
									  options=GlobalOptions)


	# This is called whenever the mural width/height or tile width/height
	# changes.
	# We recompute the total mural size in pixels and update the widgets.
	def __RecomputeTotalSize(self):
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

	# ----------------------------------------------------------------------
	# Event handling

	def __OnSizeChange(self, event):
		"""Called when tile size changes with spin controls."""
		self.__RecomputeTotalSize()
		self.drawArea.Refresh()
		self.dirty = true

	def __OnLayoutChange(self, event):
		"""Called when left/right top/bottom layout changes."""
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
		self.__RecomputeTotalSize()
		self.drawArea.Refresh()
		self.dirty = true

	# Called when hostname or first host index changes
	def __OnHostChange(self, event):
		"""Called when the host name pattern or first index changes."""
		self.HostNamePattern = self.hostText.GetValue()
		self.HostNameStart = self.hostSpin.GetValue()
		self.drawArea.Refresh()
		self.dirty = true


	def __OnPaintEvent(self, event):
		""" Respond to a request to redraw the contents of our drawing panel.
		"""
		dc = wxPaintDC(self.drawArea)
#		self.drawArea.PrepareDC(dc)  # only for scrolled windows
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
				s = MakeHostname(self.HostNamePattern, self.HostNameStart + k)
				dc.DrawText(s, x+3, y+3)
		dc.EndDrawing()

	# ----------------------------------------------------------------------
	# File I/O

	def LoadConfiguration(self, filename):
		"""Load a configuration file."""
		f = open(fileName, "r")
		if f:
			while true:
				l = f.readline()
				if not l:
					break
				if re.match("^TILE_ROWS = [0-9]+$", l):
					v = re.search("[0-9]+", l)
					self.heightControl.SetValue(int(l[v.start() : v.end()]))
				elif re.match("^TILE_COLS = [0-9]+$", l):
					v = re.search("[0-9]+", l)
					self.widthControl.SetValue(int(l[v.start() : v.end()]))
				elif re.match("^TILE_WIDTH = [0-9]+$", l):
					v = re.search("[0-9]+", l)
					self.tileWidthControl.SetValue(int(l[v.start() : v.end()]))
				elif re.match("^TILE_HEIGHT = [0-9]+$", l):
					v = re.search("[0-9]+", l)
					self.tileHeightControl.SetValue(int(l[v.start() : v.end()]))
				elif re.match("^BOTTOM_TO_TOP = [01]$", l):
					v = re.search("[01]", l)
					self.vLayoutRadio.SetSelection(int(l[v.start() : v.end()]))
				elif re.match("^RIGHT_TO_LEFT = [01]$", l):
					v = re.search("[01]", l)
					self.hLayoutRadio.SetSelection(int(l[v.start() : v.end()]))
				elif re.match("^HOSTNAME = ", l):
					# look for string in quotes
					v = re.search("\".+\"", l)
					# extract the string
					self.HostNamePattern = l[v.start()+1 : v.end()-1]
					self.hostText.SetValue(self.HostNamePattern)
				elif re.match("^FIRSTHOST = [0-9]+$", l):
					v = re.search("[0-9]+", l)
					self.HostNameStart = int(l[v.start() : v.end()])
					self.hostSpin.SetValue(self.HostNameStart)
				elif re.match("^TILESORT_", l):
					# A tilesort SPU option
					# extract the option name and value
					# parentheses in the regexp define groups
					# \"? is an optional double-quote character
					# [^\"] is any character but double-quote
					v = re.search("^TILESORT_([a-zA-Z0-9\_]+) = \"?([^\"]*)\"?", l)
					if v:
						name = v.group(1)
						value = v.group(2)
						if self.TilesortDialog.IsOption(name):
							self.TilesortDialog.SetValue(name, value)
						else:
							print "%s is not a recognized tilesort SPU option" % name
				elif re.match("^RENDER_", l):
					# A render SPU option
					v = re.search("^RENDER_([a-zA-Z0-9\_]+) = \"?([^\"]*)\"?", l)
					if v:
						name = v.group(1)
						value = v.group(2)
						if self.RenderDialog.IsOption(name):
							self.RenderDialog.SetValue(name, value)
						else:
							print "%s is not a recognized render SPU option" % name
				elif re.match("^SERVER_", l):
					# A server option
					v = re.search("^SERVER_([a-zA-Z0-9\_]+) = \"?([^\"]*)\"?", l)
					if v:
						name = v.group(1)
						value = v.group(2)
						if self.ServerDialog.IsOption(name):
							self.ServerDialog.SetValue(name, value)
						else:
							print "%s is not a recognized server option" % name
				elif re.match("^GLOBAL_", l):
					# A global option
					v = re.search("^GLOBAL_([a-zA-Z0-9\_]+) = \"?([^\"]*)\"?", l)
					if v:
						name = v.group(1)
						value = v.group(2)
						if self.GlobalDialog.IsOption(name):
							self.GlobalDialog.SetValue(name, value)
						else:
							print "%s is not a recognized global option" % name
				elif re.match("^# end of options$", l):
					# that's the end of the variables
					# save the rest of the file....
					break
				elif not re.match("\s*#", l):
					print "unrecognized line: %s" % l
			f.close()
			self.__RecomputeTotalSize()
		self.dirty = false
		self.drawArea.Refresh()

	def __WriteOptions(self, file, prefix, options, dialog):
		"""Helper function for writing config file options"""
		for (name, descrip, type, count, default, mins, maxs)  in options:
			values = dialog.GetValue(name)
			if len(values) == 1:
				valueStr = str(values[0])
			else:
				valueStr = str(values)
			if type == "INT" or type == "BOOL":
				file.write("%s_%s = %s\n" % (prefix, name, valueStr))
			elif type == "FLOAT":
				file.write("%s_%s = %s\n" % (prefix, name, valueStr))
			else:
				file.write("%s_%s = \"%s\"\n" % (prefix, name, valueStr))
		# endfor

	def SaveConfiguration(self):
		"""Save the configuration."""
		f = open(self.fileName, "w")
		if f:
			f.write("# Chromium tilesort config file\n")
			f.write("TILE_ROWS = %d\n" % self.heightControl.GetValue())
			f.write("TILE_COLS = %d\n" % self.widthControl.GetValue())
			f.write("TILE_WIDTH = %d\n" % self.tileWidthControl.GetValue())
			f.write("TILE_HEIGHT = %d\n" % self.tileHeightControl.GetValue())
			f.write("RIGHT_TO_LEFT = %d\n" % self.hLayoutRadio.GetSelection())
			f.write("BOTTOM_TO_TOP = %d\n" % self.vLayoutRadio.GetSelection())
			f.write("HOSTNAME = \"%s\"\n" % self.HostNamePattern)
			f.write("FIRSTHOST = %d\n" % self.HostNameStart)
			tilesortOptions = self.tilesortInfo[1]
			self.__WriteOptions(f, "TILESORT", tilesortOptions, self.TilesortDialog)
			renderOptions = self.renderInfo[1]
			self.__WriteOptions(f, "RENDER", renderOptions, self.RenderDialog)
			self.__WriteOptions(f, "SERVER", ServerOptions, self.ServerDialog)
			self.__WriteOptions(f, "GLOBAL", GlobalOptions, self.GlobalDialog)
			f.write("# end of options\n")
			f.write(ConfigBody)
			f.close()
		self.dirty = false

	def SetValues(self, values):
		"""Set the tilesort parameters, update widgets.
		values is a TilesortOptions object."""
		pass

	def GetValues(self, values):
		"""Retrieve the tilesort parameters.  Return a TilesortOptions object.
		"""
		return 0

	def SetMothership(self, mothership):
		"""Specify the mothership to modify.
		mothership is a Mothership object.
		"""
		pass

# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

""" graph.py

    Tool for making arbitrary Chromium configuration graphs.

"""


import string, os.path, types, random
from wxPython.wx import *
from spudialog import *
from crutils import *


#----------------------------------------------------------------------------
#                            System Constants
#----------------------------------------------------------------------------

# Our menu item IDs:

menu_SELECT_ALL_NODES   = 200
menu_DESELECT_ALL_NODES = 201
menu_DELETE_NODE        = 202
menu_CONNECT            = 203
menu_DISCONNECT         = 204
menu_LAYOUT_NODES       = 205
menu_SET_HOST           = 206
menu_SERVER_OPTIONS     = 207

menu_SELECT_ALL_SPUS    = 300
menu_DESELECT_ALL_SPUS  = 301
menu_DELETE_SPU         = 302
menu_SPU_OPTIONS        = 303

menu_HELP               = 400
menu_ABOUT              = 401


# Widget IDs
id_NewServerNode  = 3000
id_NewAppNode     = 3001
id_NewSpu         = 3002
id_NewTemplate    = 3003

# Size of the drawing page, in pixels.
PAGE_WIDTH  = 1000
PAGE_HEIGHT = 1000

SpuClasses = [ "New SPU", "Pack", "Tilesort", "Render", "Readback", "Wet", "Hiddenline", "Print", "Saveframe", "Comm", "Binaryswap", "Array", "Counter", "Dist_texture" ]

# How many servers can this SPU connect to
SpuMaxServers = { "Pack" : 1,  "Tilesort" : 1000 }

# SPUs which must be at the end of a chain, other than packers
TerminalSPUs = [ "Render", "Pack", "Tilesort" ]

Templates = [ "New Template", "Tilesort", "Sort-last" ]

WildcardPattern = "Chromium Configs (*.conf)|*.conf|All (*)|*"

AppNodeBrush = wxBrush(wxColor(55, 160, 55))
ServerNodeBrush = wxBrush(wxColor(210, 105, 135))
BackgroundColor = wxColor(90, 150, 190)

ConfigFileHeader = """
import string
import sys
sys.path.append( "../server" )
sys.path.append( "../tools" )
from mothership import *
from crutils import *

cr = CR()

"""

ConfigFileTail = """
cr.Go()
"""

# XXX this is a temporary hack.  We want to eventually retrieve this
# info automatically via an external SPU utility program.
SPUOptions = {
	"Tilesort" : [
		("broadcast", "bool", 1, false, "Broadcast Primitives"),
		("optimize_bucket", "bool", 1, true, "Optimized Bucketing"),
		("split_begin_end", "bool", 1, false, "Split glBegin/glEnd"),
		("sync_on_swap", "bool", 1, false, "Sync on SwapBuffers()")
		],
	"Render" : [
		("try_direct", "bool", 1, true, "Try Direct Rendering"),
		("force_direct", "bool", 1, true, "Force Direct Rendering"),
		("fullscreen", "bool", 1, false, "Full-screen Window"),
		("on_top", "bool", 1, false, "Display on top")
		],
	"Print" : [
		("log_file", "string", 1, "logfile", "Log file name")
		],
	"Readback" : [
		("extract_depth", "bool", 1, true, "Extract Z Zalues"),
		("extract_alpha", "bool", 1, true, "Extract Alpha Zalues"),
		("local_visualization", "bool", 1, false, "Local Visualization")
		]
}


#----------------------------------------------------------------------------
# Main window frame class

class MainFrame(wxFrame):
	""" A frame showing the contents of a single document. """

	def __init__(self, parent, id, title, fileName=None):
		"""parent, id, title are the usual wxFrame parameters.
		fileName is the name and path of a config file to load, if any.
		"""
		wxFrame.__init__(self, parent, id, title,
				 style = wxDEFAULT_FRAME_STYLE | wxWANTS_CHARS |
					 wxNO_FULL_REPAINT_ON_RESIZE)

		# Setup our menu bar.
		menuBar = wxMenuBar()

		# File menu
		self.fileMenu = wxMenu()
		self.fileMenu.Append(wxID_NEW,    "New\tCTRL-N")
		self.fileMenu.Append(wxID_OPEN,   "Open...\tCTRL-O")
		self.fileMenu.Append(wxID_CLOSE,  "Close\tCTRL-W")
		self.fileMenu.AppendSeparator()
		self.fileMenu.Append(wxID_SAVE,   "Save\tCTRL-S")
		self.fileMenu.Append(wxID_SAVEAS, "Save As...")
		self.fileMenu.Append(wxID_REVERT, "Revert...")
		self.fileMenu.AppendSeparator()
		self.fileMenu.Append(wxID_EXIT,   "Quit\tCTRL-Q")
		EVT_MENU(self, wxID_NEW,    self.doNew)
		EVT_MENU(self, wxID_OPEN,   self.doOpen)
		EVT_MENU(self, wxID_CLOSE,  self.doClose)
		EVT_MENU(self, wxID_SAVE,   self.doSave)
		EVT_MENU(self, wxID_SAVEAS, self.doSaveAs)
		EVT_MENU(self, wxID_REVERT, self.doRevert)
		EVT_MENU(self, wxID_EXIT,   self.doExit)
		menuBar.Append(self.fileMenu, "File")

		# Node menu
		self.nodeMenu = wxMenu()
		self.nodeMenu.Append(menu_SELECT_ALL_NODES,   "Select All\tCTRL-A")
		self.nodeMenu.Append(menu_DESELECT_ALL_NODES, "Deselect All")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_DELETE_NODE,        "Delete\tCTRL-D")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_CONNECT,            "Connect")
		self.nodeMenu.Append(menu_DISCONNECT,         "Disconnect")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_LAYOUT_NODES,       "Re-layout")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_SET_HOST,           "Set Host...")
		self.nodeMenu.Append(menu_SERVER_OPTIONS,     "Server Options...")
		EVT_MENU(self, menu_SELECT_ALL_NODES, self.doSelectAllNodes)
		EVT_MENU(self, menu_DESELECT_ALL_NODES, self.doDeselectAllNodes)
		EVT_MENU(self, menu_DELETE_NODE, self.doDeleteNodes)
		EVT_MENU(self, menu_CONNECT, self.doConnect)
		EVT_MENU(self, menu_DISCONNECT, self.doDisconnect)
		EVT_MENU(self, menu_LAYOUT_NODES, self.doLayoutNodes)
		EVT_MENU(self, menu_SET_HOST, self.doSetHost)
		EVT_MENU(self, menu_SERVER_OPTIONS, self.doServerOptions)
		menuBar.Append(self.nodeMenu, "Node")

		# SPU menu
		self.spuMenu = wxMenu()
		self.spuMenu.AppendSeparator()
		self.spuMenu.Append(menu_SELECT_ALL_SPUS,    "Select All ")
		self.spuMenu.Append(menu_DESELECT_ALL_SPUS,  "Deselect All")
		self.spuMenu.AppendSeparator()
		self.spuMenu.Append(menu_DELETE_SPU,         "Delete ")
		self.spuMenu.AppendSeparator()
		self.spuMenu.Append(menu_SPU_OPTIONS,        "Options...")
		EVT_MENU(self, menu_SELECT_ALL_SPUS, self.doSelectAllSPUs)
		EVT_MENU(self, menu_DESELECT_ALL_SPUS, self.doDeselectAllSPUs)
		EVT_MENU(self, menu_DELETE_SPU, self.doDeleteSPU)
		EVT_MENU(self, menu_SPU_OPTIONS, self.doSpuOptions)
		menuBar.Append(self.spuMenu, "SPU")

		# Help menu
		self.helpMenu = wxMenu()
		self.helpMenu.Append(menu_HELP,  "Introduction...")
		self.helpMenu.Append(menu_ABOUT, "About Config tool...")
		EVT_MENU(self, menu_HELP, self.doShowIntro)
		EVT_MENU(self, menu_ABOUT, self.doShowAbout)
		menuBar.Append(self.helpMenu, "Help")

		self.SetMenuBar(menuBar)

		# Install our own method to handle closing the window.  This allows us
		# to ask the user if he/she wants to save before closing the window, as
		# well as keeping track of which windows are currently open.

		EVT_CLOSE(self, self.doClose)

		# Setup our top-most panel.  This holds the entire contents of the
		# window, excluding the menu bar.

		self.topPanel = wxPanel(self, id=-1)

		toolSizer = wxBoxSizer(wxHORIZONTAL)

		# New app node button
		appChoices = ["New App Node(s)", "1 App node", "2 App nodes",
					  "3 App nodes", "4 App nodes"]
		self.newAppChoice = wxChoice(parent=self.topPanel, id=id_NewAppNode,
									  choices=appChoices)
		EVT_CHOICE(self.newAppChoice, id_NewAppNode, self.onNewAppNode)
		toolSizer.Add(self.newAppChoice, flag=wxEXPAND+wxALL, border=2)

		# New server node button
		serverChoices = ["New Server Node(s)", "1 Server node", "2 Server nodes",
						 "3 Server nodes", "4 Server nodes"]
		self.newServerChoice = wxChoice(parent=self.topPanel, id=id_NewServerNode,
									  choices=serverChoices)
		EVT_CHOICE(self.newServerChoice, id_NewServerNode, self.onNewServerNode)
		toolSizer.Add(self.newServerChoice, flag=wxEXPAND+wxALL, border=2)

		# New SPU button
		self.newSpuChoice = wxChoice(parent=self.topPanel, id=id_NewSpu,
									 choices=SpuClasses)
		EVT_CHOICE(self.newSpuChoice, id_NewSpu, self.onNewSpu)
		toolSizer.Add(self.newSpuChoice, flag=wxEXPAND+wxALL, border=2)

		# New Template button
		self.newTemplateChoice = wxChoice(parent=self.topPanel, id=id_NewTemplate,
									 choices=Templates)
		EVT_CHOICE(self.newTemplateChoice, id_NewTemplate, self.onNewTemplate)
		toolSizer.Add(self.newTemplateChoice, flag=wxEXPAND+wxALL, border=2)

		# Setup the main drawing area.
		self.drawArea = wxScrolledWindow(self.topPanel, -1,
										 style=wxSUNKEN_BORDER)
		self.drawArea.EnableScrolling(true, true)
		self.drawArea.SetScrollbars(20, 20, PAGE_WIDTH / 20, PAGE_HEIGHT / 20)
		self.drawArea.SetBackgroundColour(BackgroundColor)
		EVT_PAINT(self.drawArea, self.onPaintEvent)

		EVT_LEFT_DOWN(self.drawArea, self.onMouseEvent)
		EVT_LEFT_UP(self.drawArea, self.onMouseEvent)
		EVT_MOTION(self.drawArea, self.onMouseMotion)

		# Position everything in the window.
		topSizer = wxBoxSizer(wxVERTICAL)
		topSizer.Add(toolSizer, 0, wxTOP | wxLEFT | wxRIGHT | wxALIGN_TOP, 5)
		topSizer.Add(self.drawArea, 1, wxEXPAND)

		self.topPanel.SetAutoLayout(true)
		self.topPanel.SetSizer(topSizer)

		self.SetSizeHints(minW=500, minH=200)
		self.SetSize(wxSize(700, 400))

		self.dirty     = false
		self.fileName  = fileName
		self.Nodes = []
		self.LeftDown = 0
		self.DragStartX = 0
		self.DragStartY = 0
		self.SelectDeltaX = 0
		self.SelectDeltaY = 0

		if self.fileName != None:
			self.loadContents()

		self.UpdateMenus()

	# ----------------------------------------------------------------------
	# Node functions

	def AddNode(self, node):
		"""Add a new node to the system"""
		self.Nodes.append(node)

	def RemoveNode(self, node):
		"""Remove a node from the system"""
		if node in self.Nodes:
			self.Nodes.remove(node)

	def SelectAll(self):
		for node in self.Nodes:
			node.Select()

	def DeselectAll(self):
		for node in self.Nodes:
			node.Deselect()

	def NumSelected(self):
		"""Return number of selected nodes."""
		n = 0
		for node in self.Nodes:
			if node.IsSelected():
				n += 1
		return n

	def NumSelectedServers(self):
		"""Return number of selected server/network nodes."""
		n = 0
		for node in self.Nodes:
			if node.IsServer() and node.IsSelected():
				n += 1
		return n

	def __compareFunc(self, node1, node2):
		(x1, y1) = node1.GetPosition()
		(x2, y2) = node2.GetPosition()
		if x1 < x2:
			return -1
		elif x1 > x2:
			return 1
		else:
			if y1 < y2:
				return -1
			elif y1 > y2:
				return 1
			else:
				return 0

	def SortNodesByPosition(self, list):
		"""Return a list all the nodes sorted by position (X-major)"""
		list.sort(self.__compareFunc)

	def LayoutNodes(self):
		"""Compute reasonable window positions for all the nodes"""
		nodeColumn = {}
		nodeRow = {}
		# first, put all nodes into column 0
		for node in self.Nodes:
			nodeColumn[node] = 0
		# assign nodes to columns
		# depth-first traversal over the node graph, using a stack
		stack = []
		for node in self.Nodes:
			stack.append(node)
		while len(stack) > 0:
			# pop node
			node = stack[0]
			stack.remove(node)
			# loop over this node's children
			for server in node.GetServers():
				if server in stack:
					stack.remove(server)
				# position this server to the right of the node
				nodeColumn[server] = nodeColumn[node] + 1
				# push the server's children onto the unresolved list
				if len(server.GetServers()) > 0:
					stack.insert(0, server)

		# compute rows for nodes
		columnSize = {}
		for node in self.Nodes:
			col = nodeColumn[node]
			if col in columnSize:
				row = columnSize[col]
				columnSize[col] += 1
			else:
				row = 0
				columnSize[col] = 1
			nodeRow[node] = row
		# find tallest column
		tallest = 0
		for col in columnSize.keys():
			if columnSize[col] >= tallest:
				tallest = columnSize[col]
		# set the (x,y) positions for each node
		for node in self.Nodes:
			x = nodeColumn[node] * 200 + 10
			y = nodeRow[node] * 60 + 10
			node.SetPosition(x, y)
			node.InvalidateLayout()


	SELECT_ONE = 1
	SELECT_EXTEND = 2
	SELECT_TOGGLE = 3
	SELECT_ALL = 4
	DESELECT_ALL = 5

	def UpdateSelection(self, list, obj, mode):
		"""General purpose selection-update function.  Used both for
		nodes and SPUs.
		<list> is a list of objects
		<obj> is one object in the list, the one clicked on, if any
		<mode> may be one of SELECT_ONE, SELECT_EXTEND, SELECT_TOGGLE,
		SELECT_ALL or DESELECT_ALL
		"""
		if mode == self.SELECT_ONE:
			if obj.IsSelected():
				# do nothing
				pass
			else:
				# make obj the only one selected
				for object in list:
					if object == obj:
						object.Select()
					else:
						object.Deselect()
		elif mode == self.SELECT_EXTEND:
			obj.Select()
		elif mode == self.SELECT_TOGGLE:
			if obj.IsSelected():
				obj.Deselect()
			else:
				obj.Select()
		elif mode == self.SELECT_ALL:
			for object in list:
				object.Select()
		elif mode == self.DESELECT_ALL:
			for object in list:
				object.Deselect()
		else:
			print "bad mode in UpdateSelection"

	def GetSelectedSPUs(self):
		"""Return a list of all the selected SPUs"""
		spuList = []
		for node in self.Nodes:
			if node.IsSelected():
				for spu in node.SPUChain():
					if spu.IsSelected():
						spuList.append(spu)
		return spuList

	def NumSelectedSPUs(self):
		"""Return number of selected SPUs"""
		count = 0
		for node in self.Nodes:
			if node.IsSelected():
				for spu in node.SPUChain():
					if spu.IsSelected():
						count += 1
		return count

	#----------------------------------------------------------------------
	# Template functions

	def CreateTilesort(self):
		"""Create a tilesort configuration"""
		# XXX need an integer dialog here!!!!
		dialog = wxTextEntryDialog(self, message="Enter number of server/render nodes")
		dialog.SetTitle("Create Tilesort (sort-first) configuration")
		if dialog.ShowModal() == wxID_OK:
			numServers = int(dialog.GetValue())
			hostname = "localhost"
			self.DeselectAll()
			# Create the app node
			xPos = 5
			yPos = numServers * 70 / 2 - 20
			appNode = ApplicationNode(host=hostname)
			appNode.SetPosition(xPos, yPos)
			appNode.Select()
			tilesortSPU = SpuObject("Tilesort")
			appNode.AddSPU(tilesortSPU)
			self.AddNode(appNode)
			# Create the server nodes
			xPos = 300
			yPos = 5
			for i in range(0, numServers):
				serverNode = NetworkNode(host=hostname)
				serverNode.SetPosition(xPos, yPos)
				serverNode.Select()
				renderSPU = SpuObject("Render")
				serverNode.AddSPU(renderSPU)
				self.AddNode(serverNode)
				tilesortSPU.AddServer(serverNode)
				yPos += 70
			
		dialog.Destroy()
		return

	def CreateSortlast(self):
		"""Create a sort-last configuration"""
		# XXX need an integer dialog here!!!!
		dialog = wxTextEntryDialog(self, message="Enter number of application nodes")
		dialog.SetTitle("Create Sort-last configuration")
		if dialog.ShowModal() == wxID_OK:
			numClients = int(dialog.GetValue())
			hostname = "client##"
			self.DeselectAll()
			# Create the server/render node
			xPos = 300
			yPos = numClients * 70 / 2 - 20
			serverNode = NetworkNode(host="foobar")
			serverNode.SetPosition(xPos, yPos)
			serverNode.Select()
			renderSPU = SpuObject("Render")
			serverNode.AddSPU(renderSPU)
			self.AddNode(serverNode)
			# Create the client/app nodes
			xPos = 5
			yPos = 5
			for i in range(0, numClients):
				appNode = ApplicationNode(host=hostname)
				appNode.SetPosition(xPos, yPos)
				appNode.Select()
				readbackSPU = SpuObject("Readback")
				appNode.AddSPU(readbackSPU)
				packSPU = SpuObject("Pack")
				appNode.AddSPU(packSPU)
				self.AddNode(appNode)
				packSPU.AddServer(serverNode)
				yPos += 70
			
		dialog.Destroy()
		return


    #----------------------------------------------------------------------
	# Event handlers / callbacks

	def onPaintEvent(self, event):
		"""Drawing area repaint callback"""
		dc = wxPaintDC(self.drawArea)
		self.drawArea.PrepareDC(dc)  # only for scrolled windows
		dc.BeginDrawing()

		# draw the nodes
		for node in self.Nodes:
			if node.IsSelected():
				node.Draw(dc, self.SelectDeltaX, self.SelectDeltaY)
			else:
				node.Draw(dc)

		# draw the wires between the nodes
		pen = wxPen(wxColor(0, 0, 250))
		pen.SetWidth(2)
		dc.SetPen(pen)
		for node in self.Nodes:
			servers = node.GetServers()
			for s in servers:
				p = node.GetOutputPlugPos()
				q = s.GetInputPlugPos()
				dc.DrawLine( p[0], p[1], q[0], q[1] )
		dc.EndDrawing()

	# called when New App Node button is pressed
	def onNewAppNode(self, event):
		self.DeselectAll()
		xstart = random.randrange(10, 50, 5)
		ystart = random.randrange(50, 100, 5)
		n = self.newAppChoice.GetSelection()
		for i in range(0, n):
			node = ApplicationNode("app%d" % i)
			node.SetPosition(xstart, ystart + i * 65)
			node.Select()
			self.AddNode(node)
		self.newAppChoice.SetSelection(0)
		self.drawArea.Refresh()
		self.UpdateMenus()

	# called when New Server Node button is pressed
	def onNewServerNode(self, event):
		self.DeselectAll()
		xstart = random.randrange(250, 300, 5)
		ystart = random.randrange(50, 100, 5)
		n = self.newServerChoice.GetSelection()
		for i in range(0, n):
			node = NetworkNode("cr%d" % i)
			node.SetPosition(xstart, ystart + i * 65)
			node.Select()
			self.AddNode(node)
		self.newServerChoice.SetSelection(0)
		self.drawArea.Refresh()
		self.UpdateMenus()

	def onNewSpu(self, event):
		"""New SPU button callback"""
		# add a new SPU to all selected nodes
		i = self.newSpuChoice.GetSelection()
		if i <= 0:
			return # didn't really select an SPU class
		for node in self.Nodes:
			if node.IsSelected():
				# we'll insert before the first selected SPU, or at the
				# end if no SPUs are selected
				pos = node.GetFirstSelectedSPUPos()
				# get the predecessor SPU
				if pos == -1:
					pred = node.LastSPU()
				elif pos > 0:
					pred = node.GetSPU(pos - 1)
				else:
					pred = 0
				# check if it's legal to put this SPU after the predecessor
				if pred and pred.IsTerminal():
					self.Notify("You can't put a %s SPU after a %s SPU." %
								(SpuClasses[i], pred.Name()))
					break
				# check if it's legal to put this SPU before another
				if pos >= 0 and SpuClasses[i] in TerminalSPUs:
					self.Notify("You can't insert a %s SPU before a %s SPU." %
								(SpuClasses[i], node.GetSPU(pos).Name()))
					break
				# OK, we're all set, add the SPU
				s = SpuObject( SpuClasses[i] )
				node.AddSPU(s, pos)
		self.drawArea.Refresh()
		self.newSpuChoice.SetSelection(0)

	def onNewTemplate(self, event):
		"""New Template button callback"""
		t = self.newTemplateChoice.GetSelection()
		if t == 1:
			self.CreateTilesort()
		elif t == 2:
			self.CreateSortlast()
		self.newTemplateChoice.SetSelection(0)
		self.drawArea.Refresh()
		self.UpdateMenus()

	# Called when the left mouse button is pressed or released.
	def onMouseEvent(self, event):
		(x,y) = self.drawArea.CalcUnscrolledPosition(event.GetX(), event.GetY())
		# First, determine if we're clicking on an object
		# iterate backward through the object list so we get the topmost one
		hitNode = 0
		hitSPU = 0
		for i in range(len(self.Nodes) - 1, -1, -1):
			node = self.Nodes[i]
			p = node.PickTest(x,y)
			if p >= 1:
				hitNode = node
				if p > 1 and event.LeftDown():
					hitSPU = hitNode.GetSPU(p - 2)
				break
			# endif
		# endfor

		self.LeftDown = event.LeftDown()

		# Now handle selection/deselection
		if self.LeftDown:
			# mouse down
			if hitNode:
				# hit a node (and maybe an SPU)
				self.DragStartX = x
				self.DragStartY = y
				self.SelectDeltaX = 0
				self.SelectDeltaY = 0
				if event.ControlDown():
					mode = self.SELECT_TOGGLE
				elif event.ShiftDown():
					mode = self.SELECT_EXTEND
				else:
					mode = self.SELECT_ONE
				if hitSPU:
					# also hit an SPU
					self.UpdateSelection(hitNode.SPUChain(), hitSPU, mode)
					self.UpdateSelection(self.Nodes, hitNode, self.SELECT_EXTEND)
				else:
					self.UpdateSelection(hitNode.SPUChain(), 0, self.DESELECT_ALL)
					self.UpdateSelection(self.Nodes, hitNode, mode)
			elif event.ControlDown() or event.ShiftDown():
				self.LeftDown = 0
			else: #if not 
				# didn't hit an SPU or a node
				for node in self.Nodes:
					self.UpdateSelection(node.SPUChain(), 0, self.DESELECT_ALL)
				self.UpdateSelection(self.Nodes, 0, self.DESELECT_ALL)
		else:
			# mouse up
			for node in self.Nodes:
				if node.IsSelected():
					p = node.GetPosition()
					x = p[0]
					y = p[1]
					x += self.SelectDeltaX
					y += self.SelectDeltaY
					node.SetPosition(x, y)
			self.SelectDeltaX = 0
			self.SelectDeltaY = 0
			self.LeftDown = 0

		self.UpdateMenus()
		self.drawArea.Refresh()

	# Called when the mouse moves.  We only really need to call this when a
	# mouse button is also pressed, but I don't see a way to specify that with
	# wxWindows as you can do with X.
	def onMouseMotion(self, event):
		if event.LeftIsDown() and self.LeftDown:
			# SelectDeltaX/Y is added to the position of selected objects
			(x,y) = self.drawArea.CalcUnscrolledPosition(event.GetX(), event.GetY())
			self.SelectDeltaX = x - self.DragStartX
			self.SelectDeltaY = y - self.DragStartY

			#self.SelectDeltaX = event.GetX() - self.DragStartX
			#self.SelectDeltaY = event.GetY() - self.DragStartY
			# 5-pixel threshold before starting movement
			if abs(self.SelectDeltaX) < 5 and abs(self.SelectDeltaY) < 5:
				self.SelectDeltaX = 0
				self.SelectDeltaY = 0

			anySelected = 0
			for node in self.Nodes:
				if node.IsSelected():
					anySelected = 1
					break
			if anySelected:
				self.drawArea.Refresh()

	# ----------------------------------------------------------------------
	# File menu callbacks

	def doNew(self, event):
		"""File / New callback"""
		global _docList
		newFrame = MainFrame(None, -1, "Chromium Configuration Tool")
		newFrame.Show(TRUE)
		_docList.append(newFrame)


	def doOpen(self, event):
		"""File / Open callback"""
		global _docList

		curDir = os.getcwd()
		fileName = wxFileSelector("Open File", default_extension="conf",
					  flags = wxOPEN | wxFILE_MUST_EXIST)
		if fileName == "":
			return
		fileName = os.path.join(os.getcwd(), fileName)
		os.chdir(curDir)

		title = os.path.basename(fileName)

		if (self.fileName == None) and (len(self.contents) == 0):
			# Load contents into current (empty) document.
			self.fileName = fileName
			self.SetTitle(os.path.basename(fileName))
			self.loadContents()
		else:
			# Open a new frame for this document.
			newFrame = MainFrame(None, -1, os.path.basename(fileName),
						fileName=fileName)
			newFrame.Show(true)
			_docList.append(newFrame)


	def doClose(self, event):
		"""File / Close callback"""
		global _docList

		if self.dirty:
			if not self.askIfUserWantsToSave("closing"):
				return

		_docList.remove(self)
		self.Destroy()


	def doSave(self, event):
		"""File / Save callback"""
		if self.fileName != None:
			self.saveGraph()


	def doSaveAs(self, event):
		"""File / Save As callback"""
		if self.fileName == None:
			default = ""
		else:
			default = self.fileName

		curDir = os.getcwd()
		fileName = wxFileSelector("Save File As",
					  default_filename=default,
					  default_extension="conf",
					  wildcard=WildcardPattern,
					  flags = wxSAVE | wxOVERWRITE_PROMPT)
		if fileName == "":
			return # User cancelled.
		fileName = os.path.join(os.getcwd(), fileName)
		os.chdir(curDir)

		title = os.path.basename(fileName)
		self.SetTitle(title)

		self.fileName = fileName
		self.saveGraph()


	def doRevert(self, event):
		""" Respond to the "Revert" menu command.
		"""
		if not self.dirty:
			return

		if wxMessageBox("Discard changes made to this document?", "Confirm",
				style = wxOK | wxCANCEL | wxICON_QUESTION,
				parent=self) == wxCANCEL:
			return
		self.loadContents()


	def doExit(self, event):
		""" Respond to the "Quit" menu command.
		"""
		global _docList, _app
		for doc in _docList:
			if not doc.dirty: continue
			doc.Raise()
			if not doc.askIfUserWantsToSave("quitting"):
				return
			_docList.remove(doc)
			doc.Destroy()

		_app.ExitMainLoop()

	# ----------------------------------------------------------------------
	# Node menu callbacks

	def doSelectAllNodes(self, event):
		"""Node / Select All callback"""
		for node in self.Nodes:
			node.Select()
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doDeselectAllNodes(self, event):
		"""Node / Deselect All callback"""
		for node in self.Nodes:
			node.Deselect()
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doDeleteNodes(self, event):
		"""Node / Delete Nodes callback"""
		# Ugh, I can't find a Python counterpart to the C++ STL's remove_if()
		# function.
		# Have to make a temporary list of the objects to delete so we don't
		# screw-up the iteration as we remove things
		deleteList = []
		for node in self.Nodes:
			if node.IsSelected():
				deleteList.append(node)
		# loop over nodes again to remove server connections
		for node in self.Nodes:
			for server in node.GetServers():
				if server.IsSelected():
					node.LastSPU().RemoveServer(server)
		# now delete the objects in the deleteList
		for node in deleteList:
			 self.Nodes.remove(node)
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doConnect(self, event):
		"""Node / Connect callback"""
		# Make list of packer(app and net) nodes and server nodes
		netPackerNodes = []
		appPackerNodes = []
		serverNodes = []
		for node in self.Nodes:
			if node.IsSelected():
				if node.HasAvailablePacker():
					if node.IsServer():
						netPackerNodes.append(node)
					else:
						appPackerNodes.append(node)
				elif node.IsServer():
					serverNodes.append(node)
		#print "appPackerNodes: %d" % len(appPackerNodes)
		#print "netPackerNodes: %d" % len(netPackerNodes)
		#print "serverNodes: %d" % len(serverNodes)

		if len(appPackerNodes) == 0 and len(netPackerNodes) == 0:
			#print "no packers!"
			return

		# see if we need to reassign any packers as servers.
		# solving this situation is done via an ad hoc heuristic.
		if len(serverNodes) == 0:
			# reassign a network packer as a server
			if len(netPackerNodes) > 1:
				# sort the packer nodes by position
				self.SortNodesByPosition(netPackerNodes)
				# look if leftmost node has a tilesorter
				leftMost = netPackerNodes[0]
				if SpuMaxServers[leftMost.LastSPU().Name()] > 1:
					# leftMost node is tilesorter
					serverNodes = netPackerNodes
					serverNodes.remove(leftMost)
					netPackerNodes = [ leftMost ]
				else:
					# find rightmost netPacker node
					rightMost = netPackerNodes[-1]
					serverNodes.append(rightMost)
					netPackerNodes.remove(rightMost)
			else:
				#print "no packers 2!"
				return
		packerNodes = appPackerNodes + netPackerNodes

		#print "ending: packerNodes: %d" % len(packerNodes)
		#print "ending: serverNodes: %d" % len(serverNodes)
		
		# this is useful special case to look for
		if len(packerNodes) == len(serverNodes):
			oneToOne = 1
		else:
			oneToOne = 0

		# Now wire-up the connections from packers to servers
		for packer in packerNodes:
			for server in serverNodes:
				if packer.HasAvailablePacker() and not server.HasChild(packer):
					packer.LastSPU().AddServer(server)
					if not packer.HasAvailablePacker() and oneToOne:
						# nobody else can connect to this server now
						serverNodes.remove(server)
		# Done!
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doDisconnect(self, event):
		"""Node / Disconnect callback"""
		for node in self.Nodes:
			if node.IsSelected():
				servers = node.GetServers()
				# make list of servers to remove
				removeList = []
				for s in servers:
					if s.IsSelected():
						removeList.append(s)
				# now remove them
				for s in removeList:
					node.LastSPU().RemoveServer(s)
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doLayoutNodes(self, event):
		"""Node / Layout Nodes callback"""
		self.LayoutNodes()
		self.drawArea.Refresh()

	def doSetHost(self, event):
		"""Node / Set Host callback"""
		dialog = wxTextEntryDialog(self, message="Enter the hostname for the selected nodes")
		dialog.SetTitle("Chromium host")
		if dialog.ShowModal() == wxID_OK:
			for node in self.Nodes:
				if node.IsSelected():
					node.SetHost(dialog.GetValue())
		dialog.Destroy()
		self.drawArea.Refresh()

	def doServerOptions(self, event):
		"""Node / Server Options callback"""
		# XXX display the server options dialog
		return


	# ----------------------------------------------------------------------
	# SPU menu callbacks

	def doSelectAllSPUs(self, event):
		"""SPU / Select All callback"""
		for node in self.Nodes:
			if node.IsSelected():
				for spu in node.SPUChain():
					spu.Select()
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doDeselectAllSPUs(self, event):
		"""SPU / Deselect All callback"""
		for node in self.Nodes:
			if node.IsSelected():
				for spu in node.SPUChain():
					spu.Deselect()
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doDeleteSPU(self, event):
		"""Node / Delete SPU callback"""
		# loop over all nodes, selected or not
		for node in self.Nodes:
			# make list of SPUs to delete
			removeList = []
			for spu in node.SPUChain():
				if spu.IsSelected():
					removeList.append(spu)
					node.InvalidateLayout()
			# now remove
			for spu in removeList:
				node.RemoveSPU(spu)
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doSpuOptions(self, event):
		"""SPU / SPU Options callback"""
		# find first selected SPU
		spuList = self.GetSelectedSPUs()
		if len(spuList) > 0:
			name = spuList[0].Name()
			if name in SPUOptions.keys():
				spuOpts = SPUOptions[name]
				dialog = SPUDialog(parent=NULL, id=-1,
								   title=name + " SPU Options",
								   options = spuOpts)
				dialog.ShowModal()
		return
		
	# ----------------------------------------------------------------------
	# Help menu callbacks

	def doShowIntro(self, event):
		"""Help / Introduction callback"""
		dialog = wxDialog(self, -1, "Introduction") # ,
				  #style=wxDIALOG_MODAL | wxSTAY_ON_TOP)
		#dialog.SetBackgroundColour(wxWHITE)

		panel = wxPanel(dialog, -1)
		#panel.SetBackgroundColour(wxWHITE)

		panelSizer = wxBoxSizer(wxVERTICAL)

		text = wxStaticText(parent=panel, id=-1, label=
			"Use the New App Node(s) button to create new application nodes.\n"
			"Use the New Server Node(s) button to create new server nodes.\n"
			"Use the New SPU button to add an SPU to the selected nodes.\n"
			"Use the mouse to select and move nodes.\n"
			"Shift-click extends the selection.\n"
			"Control-click toggles the selection.\n")

		btnOK = wxButton(panel, wxID_OK, "OK")

		panelSizer.Add(text, 0, wxALIGN_CENTRE)
		panelSizer.Add(10, 10) # Spacer.
		panelSizer.Add(btnOK, 0, wxALL | wxALIGN_CENTRE, 5)

		panel.SetAutoLayout(true)
		panel.SetSizer(panelSizer)
		panelSizer.Fit(panel)

		topSizer = wxBoxSizer(wxHORIZONTAL)
		topSizer.Add(panel, 0, wxALL, 10)

		dialog.SetAutoLayout(true)
		dialog.SetSizer(topSizer)
		topSizer.Fit(dialog)

		dialog.Centre()

		btn = dialog.ShowModal()
		dialog.Destroy()


	def doShowAbout(self, event):
		"""Help / About callback"""
		dialog = wxDialog(self, -1, "About")
		dialog.SetBackgroundColour(wxWHITE)

		panel = wxPanel(dialog, -1)
		panelSizer = wxBoxSizer(wxVERTICAL)

		text = wxStaticText(parent=panel, id=-1, label=
			"Chromium configuration tool\n"
			"Version 0.0\n")

		btnOK = wxButton(panel, wxID_OK, "OK")

		panelSizer.Add(text, 0, wxALIGN_CENTRE)
		panelSizer.Add(10, 10) # Spacer.
		panelSizer.Add(btnOK, 0, wxALL | wxALIGN_CENTRE, 5)

		panel.SetAutoLayout(true)
		panel.SetSizer(panelSizer)
		panelSizer.Fit(panel)

		topSizer = wxBoxSizer(wxHORIZONTAL)
		topSizer.Add(panel, 0, wxALL, 10)

		dialog.SetAutoLayout(true)
		dialog.SetSizer(topSizer)
		topSizer.Fit(dialog)

		dialog.Centre()

		btn = dialog.ShowModal()
		dialog.Destroy()


	# ----------------------------------------------------------------------
	def UpdateMenus(self):
		"""Enable/disable menu items as needed."""
		# XXX the enable/disable state for connect/disconnect is more
		# complicated than this.  So is the newSpuChoice widget state.
		if self.NumSelected() > 0:
			self.nodeMenu.Enable(menu_DELETE_NODE, 1)
			self.nodeMenu.Enable(menu_CONNECT, 1)
			self.nodeMenu.Enable(menu_DISCONNECT, 1)
			self.nodeMenu.Enable(menu_SET_HOST, 1)
			self.newSpuChoice.Enable(1)
		else:
			self.nodeMenu.Enable(menu_DELETE_NODE, 0)
			self.nodeMenu.Enable(menu_CONNECT, 0)
			self.nodeMenu.Enable(menu_DISCONNECT, 0)
			self.nodeMenu.Enable(menu_SET_HOST, 0)
			self.newSpuChoice.Enable(0)
		if self.NumSelectedServers() > 0:
			self.nodeMenu.Enable(menu_SERVER_OPTIONS, 1)
		else:
			self.nodeMenu.Enable(menu_SERVER_OPTIONS, 0)
		if len(self.Nodes) > 0:
			self.nodeMenu.Enable(menu_SELECT_ALL_NODES, 1)
			self.nodeMenu.Enable(menu_DESELECT_ALL_NODES, 1)
			self.nodeMenu.Enable(menu_LAYOUT_NODES, 1)
			self.spuMenu.Enable(menu_SELECT_ALL_SPUS, 1)
		else:
			self.nodeMenu.Enable(menu_SELECT_ALL_NODES, 0)
			self.nodeMenu.Enable(menu_DESELECT_ALL_NODES, 0)
			self.nodeMenu.Enable(menu_LAYOUT_NODES, 0)
			self.spuMenu.Enable(menu_SELECT_ALL_SPUS, 0)
		if self.NumSelectedSPUs() > 0:
			self.spuMenu.Enable(menu_DESELECT_ALL_SPUS, 1)
			self.spuMenu.Enable(menu_DELETE_SPU, 1)
			self.spuMenu.Enable(menu_SPU_OPTIONS, 1)
		else:
			self.spuMenu.Enable(menu_DESELECT_ALL_SPUS, 0)
			self.spuMenu.Enable(menu_DELETE_SPU, 0)
			self.spuMenu.Enable(menu_SPU_OPTIONS, 0)

	# Display a dialog with a message and OK button.
	def Notify(self, msg):
		dialog = wxMessageDialog(parent=self, message=msg,
								 caption="Hey!", style=wxOK)
		dialog.ShowModal()


	#----------------------------------------------------------------------
	# File I/O

	def loadContents(self):
		"""Load a graph from a file"""
		self.dirty = false

		self.drawArea.Refresh()


	def saveGraph(self):
		"""Save the Chromium configuration to a file."""
		print "Saving graph!"
		f = open(self.fileName, "w")
		if f:
			# file header
			f.write("# Chromium configuration produced by graph.py\n")
			f.write(ConfigFileHeader)
			# write the nodes and SPUs
			nodeNames = {}
			spuNames = {}
			n = 0
			s = 0
			for node in self.Nodes:
				nodeNames[node] = "node%d" % n
				if node.IsServer():
					f.write("node%d = crNetworkNode('%s')\n" % (n, node.Host()))
				else:
					f.write("node%d = crApplicationNode('%s')\n" % (n, node.Host()))
				# write the node's SPUs
				for spu in node.SPUChain():
					spuNames[spu] = "spu%d" % s
					f.write("spu%d = SPU('%s')\n" % (s, spu.Name()))
					f.write("node%d.AddSPU(spu%d)\n" % (n, s))
					f.write("#set spu options here\n")
					s += 1
				n += 1
				f.write("\n")
			# add servers to tilesort/packer SPUs
			for node in self.Nodes:
				lastSPU = node.LastSPU()
				if lastSPU:
					for server in lastSPU.GetServers():
						f.write("%s.AddServer(%s)\n" % (spuNames[lastSPU], nodeNames[server]))
			# endfor
			f.write("\n")
			# add nodes to mothership
			for node in self.Nodes:
				f.write("cr.AddNode(%s)\n" % nodeNames[node])
			# tail of file
			f.write(ConfigFileTail)
			f.close()
		else:
			print "Error opening %s" % self.fileName
		self.dirty = false


	def askIfUserWantsToSave(self, action):
		""" Give the user the opportunity to save the current document.
			'action' is a string describing the action about to be taken.  If
			the user wants to save the document, it is saved immediately.  If
			the user cancels, we return false.
		"""
		if not self.dirty:
			return true # Nothing to do.

		response = wxMessageBox("Save changes before " + action + "?",
					"Confirm", wxYES_NO | wxCANCEL, self)

		if response == wxYES:
			if self.fileName == None:
				fileName = wxFileSelector("Save File As",
										  default_extension="conf",
										  wildcard=WildcardPattern,
										  flags = wxSAVE | wxOVERWRITE_PROMPT)
				if fileName == "":
					return false # User cancelled.
				self.fileName = fileName

			self.saveGraph()
			return true
		elif response == wxNO:
			return true # User doesn't want changes saved.
		elif response == wxCANCEL:
			return false # User cancelled.


#----------------------------------------------------------------------------

class SpuObject:
	def __init__(self, name):
		self.__X = 0
		self.__Y = 0
		self.__Name = name
		self.__Width = 0
		self.__Height = 30
		self.__IsSelected = 0
		self.__Port = 7000
		self.__Protocol = "tcpip"
		self.__Servers = []
		self.__OutlinePen = wxPen(wxColor(0,0,0), width=1, style=0)
		self.__FillBrush = wxLIGHT_GREY_BRUSH

	def IsPacker(self):
		"""Return true if this SPU has a packer"""
		if self.__Name in SpuMaxServers.keys():
			return 1
		else:
			return 0

	def IsTerminal(self):
		"""Return true if this SPU has to be the last in a chain (a terminal)
		"""
		return self.__Name in TerminalSPUs

	def Select(self):
		self.__IsSelected = 1

	def Deselect(self):
		self.__IsSelected = 0

	def IsSelected(self):
		return self.__IsSelected

	def CanAddServer(self):
		"""Test if a server can be added to this SPU.
		Return "OK" if so, else return reason why not.
		"""
		if not self.IsPacker():
			return "This SPU doesn't have a command packer"
		if len(self.__Servers) >= SpuMaxServers[self.__Name]:
			return "This SPU is limited to %d server(s)" % SpuMaxServers[self.__Name]
		return "OK"

	def AddServer(self, serverNode, protocol='tcpip', port=7000):
		"""Add a server to this SPU.  The SPU must have a packer!"""
		if self.IsPacker() and not serverNode in self.__Servers and len(self.__Servers) < SpuMaxServers[self.__Name]:
			self.__Servers.append(serverNode)
			self.__Protocol = protocol
			self.__Port = port
		else:
			print "AddServer() failed!"

	def RemoveServer(self, serverNode):
		if serverNode in self.__Servers:
			self.__Servers.remove(serverNode)

	def GetServers(self):
		"""Return the list of servers for this SPU.
		For a pack SPU the list will contain zero or one server.
		For a tilesort SPU the list will contain zero or more servers.
		Other SPU classes have no servers.
		"""
		return self.__Servers

	def Name(self):
		return self.__Name

	def SetPosition(self, x, y):
		self.__X = x
		self.__Y = y

	def GetPosition(self):
		return (self.__X, self.__Y)

	def GetWidth(self):
		return self.__Width

	def GetHeight(self):
		return self.__Height

	def PickTest(self, x, y):
		if x >= self.__X and x < self.__X + self.__Width and y >= self.__Y and y < self.__Y + self.__Height:
			return 1
		else:
			return 0

	def Layout(self, dc):
		"""Compute width and height for drawing this SPU"""
		(w, h) = dc.GetTextExtent(self.__Name)
		self.__Width = w + 8
		self.__Height = h + 8
		
	def Draw(self, dc):
		"""Draw this SPU as a simple labeled box"""
		dc.SetBrush(self.__FillBrush)
		if self.__IsSelected:
			self.__OutlinePen.SetWidth(3)
		else:
			self.__OutlinePen.SetWidth(1)
		dc.SetPen(self.__OutlinePen)
		# if width is zero, compute it now based on the text width
		if self.__Width == 0:
			self.Layout(dc)
		# draw the SPU as rectangle with text label
		dc.DrawRectangle(self.__X, self.__Y, self.__Width, self.__Height)
		dc.DrawText(self.__Name, self.__X + 4, self.__Y + 4)
		if self.IsPacker():
			# draw the input socket (a little black rect)
			dc.SetBrush(wxBLACK_BRUSH)
			dc.DrawRectangle(self.__X + self.__Width,
							 self.__Y + self.__Height/2 - 4, 4, 8)
		elif self.__Name in TerminalSPUs:
			# draw a thick right edge on the box??
			pass

class Node:
	""" The graphical representation of a Cr node (app or network).
	This is the base class for the ServerNode and NetworkNode classes.
	"""

	def __init__(self, host="localhost", isServer = 0):
		self.__X = 0
		self.__Y = 0
		self.__Width = 0
		self.__Height = 0
		self.__SpuChain = []
		self.__IsServer = isServer
		self.__IsSelected = 0
		self.__InputPlugPos = (0,0)
		self.SetHost(host)

	def IsAppNode(self):
		"""Return true if this is an app node, false otherwise."""
		return not self.__IsServer

	def IsServer(self):
		"""Return true if this is a server, false otherwise."""
		return self.__IsServer

	def HasAvailablePacker(self):
		"""Return true if we can connect a server to this node."""
		if len(self.__SpuChain) >= 1 and self.LastSPU().CanAddServer() == "OK":
			return 1
		else:
			return 0

	def HasPacker(self):
		"""Return true if the last SPU has a packer."""
		if len(self.__SpuChain) >= 1 and self.LastSPU().IsPacker():
			return 1
		else:
			return 0

	def HasChild(self, childCandidate):
		"""Test if childCandidate is a down-stream child (server) of this node
		"""
		if childCandidate in self.GetServers():
			return 1
		else:
			for server in self.GetServers():
				return server.HasChild(childCandidate)
			return 0

	def SetHost(self, hostname):
		self.__Host = hostname
		if self.__IsServer:
			self.__Label = "Server node host=" + hostname
		else:
			self.__Label = "App node host=" + hostname
		self.InvalidateLayout()

	def Host(self):
		return self.__Host

	def Select(self):
		self.__IsSelected = 1

	def Deselect(self):
		self.__IsSelected = 0

	def IsSelected(self):
		return self.__IsSelected

	def NumSPUs(self):
		"""Return number of SPUs in the chain"""
		return len(self.__SpuChain)

	def LastSPU(self):
		"""Return the last SPU in this node's SPU chain"""
		if len(self.__SpuChain) == 0:
			return 0
		else:
			return self.__SpuChain[-1]

	def GetSPU(self, pos):
		"""Return this node's SPU chain"""
		assert pos >= 0
		assert pos < len(self.__SpuChain)
		return self.__SpuChain[pos]

	def SPUChain(self):
		"""Return this node's SPU chain"""
		return self.__SpuChain

	def GetFirstSelectedSPUPos(self):
		"""Return the position (index) of this node's first selected SPU"""
		pos = 0
		for spu in self.__SpuChain:
			if spu.IsSelected():
				return pos
			pos += 1
		return -1

	def AddSPU(self, s, pos = -1):
		"""Add a new SPU at the given position (-1 = the end)"""
		if pos < 0:
			# add at tail
			self.__SpuChain.append(s)
		else:
			# insert at [pos]
			assert pos >= 0
			assert pos <= len(self.__SpuChain)
			self.__SpuChain.insert(pos, s)
		self.InvalidateLayout()

	def RemoveSPU(self, spu):
		"""Remove an SPU from the node's SPU chain"""
		if spu in self.__SpuChain:
			self.__SpuChain.remove(spu)
		else:
			print "Problem spu not in spu chain!"

	def GetServers(self):
		"""Return a list of servers that the last SPU (a packing SPU) are
		connected to."""
		if self.NumSPUs() > 0:
			return self.LastSPU().GetServers()
		else:
			return []

	def GetPosition(self):
		return (self.__X, self.__Y)

	def SetPosition(self, x, y):
		self.__X = x
		self.__Y = y

	# Return the (x,y) coordinate of the node's input socket
	def GetInputPlugPos(self):
		assert self.__IsServer
		return self.__InputPlugPos

	# Return the (x,y) coordinate of the node's output socket
	def GetOutputPlugPos(self):
		assert self.NumSPUs() > 0
		last = self.LastSPU()
		assert last.IsPacker()
		(x, y) = last.GetPosition()
		x += last.GetWidth()
		y += last.GetHeight() / 2
		return (x, y)

	def InvalidateLayout(self):
		self.__Width = 0

	def Layout(self, dc):
		"""Compute width and height for drawing this node"""
		self.__Width = 5
		for spu in self.__SpuChain:
			spu.Layout(dc)
			self.__Width += spu.GetWidth() + 2
		self.__Width += 8
		(w, h) = dc.GetTextExtent(self.__Label)
		if self.__Width < w + 8:
			self.__Width = w + 8
		if self.__Width < 100:
			self.__Width = 100
		if self.__Height == 0:
			self.__Height = int(h * 3.5)

	def Draw(self, dc, dx=0, dy=0):
		"""Draw this node.  (dx,dy) are the temporary translation values
		used when a mouse drag is in progress."""
		# setup the brush and pen
		if self.__IsServer:
			dc.SetBrush(ServerNodeBrush)
		else:
			dc.SetBrush(AppNodeBrush)
		p = wxPen(wxColor(0,0,0), width=1, style=0)
		if self.__IsSelected:
			p.SetWidth(3)
		dc.SetPen(p)
		x = self.__X + dx
		y = self.__Y + dy

		if self.__Width == 0 or self.__Height == 0:
			self.Layout(dc)

		# draw the node's box
		dc.DrawRectangle(x, y, self.__Width, self.__Height)
		if self.__IsServer:
			dc.DrawText(self.__Label, x + 4, y + 4)
			# draw the unpacker plug
			px = x - 4
			py = y + self.__Height / 2
			self.__InputPlugPos = (px, py)
			dc.SetBrush(wxBLACK_BRUSH)
			dc.DrawRectangle(px, py - 4, 4, 8)
		else:
			dc.DrawText(self.__Label, x + 4, y + 4)

		# draw the SPUs
		x = x + 5
		y = y + 20
		for spu in self.__SpuChain:
			spu.SetPosition(x, y)
			spu.Draw(dc)
			x = x + spu.GetWidth() + 2


	def PickTest(self, x, y):
		"""Return 0 if this node is not picked.
		Return 1 if the node was picked, but not an SPU
		Return n if the nth SPU was picked.
		"""
		# try the SPUs first
		i = 0
		for spu in self.__SpuChain:
			if spu.PickTest(x,y):
				return 2 + i
			i = i + 1
		# now try the node itself
		if x >= self.__X and x < self.__X + self.__Width and y >= self.__Y and y < self.__Y + self.__Height:
			return 1
		else:
			return 0

class NetworkNode(Node):
	"""A CRNetworkNode object"""
	def __init__(self, host="localhost"):
		Node.__init__(self, host, isServer=1)

class ApplicationNode(Node):
	"""A CRApplicationNode object"""
	def __init__(self, host="localhost"):
		Node.__init__(self, host, isServer=0)


#----------------------------------------------------------------------------

class ConfigApp(wxApp):
	""" The main application object.
	"""
	def OnInit(self):
		""" Initialise the application.
		"""
		wxInitAllImageHandlers()

		global _docList
		_docList = []

		if len(sys.argv) == 1:
			# No file name was specified on the command line -> start with a
			# blank document.
			frame = MainFrame(None, -1, "Chromium Configuration Tool")
			frame.Centre()
			frame.Show(TRUE)
			_docList.append(frame)
		else:
			# Load the file(s) specified on the command line.
			for arg in sys.argv[1:]:
				fileName = os.path.join(os.getcwd(), arg)
				if os.path.isfile(fileName):
					frame = MainFrame(None, -1,
						 os.path.basename(fileName),
						 fileName=fileName)
					frame.Show(TRUE)
					_docList.append(frame)

		return TRUE

#----------------------------------------------------------------------------

class ExceptionHandler:
	""" A simple error-handling class to write exceptions to a text file.

	    Under MS Windows, the standard DOS console window doesn't scroll and
	    closes as soon as the application exits, making it hard to find and
	    view Python exceptions.  This utility class allows you to handle Python
	    exceptions in a more friendly manner.
	"""

	def __init__(self):
		""" Standard constructor.
		"""
		self._buff = ""
		if os.path.exists("errors.txt"):
			os.remove("errors.txt") # Delete previous error log, if any.


	def write(self, s):
		""" Write the given error message to a text file.

			Note that if the error message doesn't end in a carriage return, we
			have to buffer up the inputs until a carriage return is received.
		"""
		if (s[-1] != "\n") and (s[-1] != "\r"):
			self._buff = self._buff + s
			return

		try:
			s = self._buff + s
			self._buff = ""

			if s[:9] == "Traceback":
			# Tell the user than an exception occurred.
				wxMessageBox("An internal error has occurred.\nPlease " + \
					 "refer to the 'errors.txt' file for details.",
					 "Error", wxOK | wxCENTRE | wxICON_EXCLAMATION)

			f = open("errors.txt", "a")
			f.write(s)
			f.close()
		except:
			pass # Don't recursively crash on errors.

#----------------------------------------------------------------------------

def main():
	""" Start up the configuration tool.
	"""
	global _app

	# Redirect python exceptions to a log file.

	sys.stderr = ExceptionHandler()

	# Create and start the Tilesort application.

	_app = ConfigApp(0)
	_app.MainLoop()


if __name__ == "__main__":
	main()


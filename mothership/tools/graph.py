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


import string, os.path, types, re, random, copy
from wxPython.wx import *

import crtypes, crutils
import templates, configio
import spudialog, intdialog, hostdialog, tiledialog


#----------------------------------------------------------------------
# Constants

menu_UNDO               = 100
menu_REDO               = 101

menu_SELECT_ALL_NODES   = 200
menu_DESELECT_ALL_NODES = 201
menu_CUT_NODE           = 202
menu_COPY_NODE          = 203
menu_PASTE_NODE         = 204
menu_DELETE_NODE        = 205
menu_CONNECT            = 206
menu_DISCONNECT         = 207
menu_SET_HOST           = 208
menu_SET_COUNT          = 209
menu_SPLIT_NODES        = 210
menu_MERGE_NODES        = 211
menu_SERVER_OPTIONS     = 212
menu_SERVER_TILES       = 213

menu_SELECT_ALL_SPUS    = 300
menu_DESELECT_ALL_SPUS  = 301
menu_DELETE_SPU         = 302
menu_SPU_OPTIONS        = 303

menu_LAYOUT_NODES       = 400
menu_GRAPH              = 401
menu_TILESORT           = 402
menu_LIGHTNING2         = 403

menu_APP_OPTIONS        = 500
menu_APP_RUN            = 501
menu_APP_STOP           = 502

menu_HELP               = 600
menu_ABOUT              = 601


# Widget IDs
id_NewServerNode  = 3000
id_NewAppNode     = 3001
id_NewSpu         = 3002
id_NewTemplate    = 3003
id_TemplateOptions = 3004

# Size of the drawing page, in pixels.
PAGE_WIDTH  = 1000
PAGE_HEIGHT = 1000

TitleString = "Chromium Configuration Tool"

WildcardPattern = "Chromium Configs (*.conf)|*.conf|All (*)|*"

AppNodeBrush = wxBrush(wxColor(55, 160, 55))
ServerNodeBrush = wxBrush(wxColor(210, 105, 135))
BackgroundColor = wxColor(90, 150, 190)



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

		# Edit menu
		self.editMenu = wxMenu()
		self.editMenu.Append(menu_UNDO,   "Undo\tCTRL-Z")
		self.editMenu.Append(menu_REDO,   "Redo\tSHIFT-CTRL-Z")
		EVT_MENU(self, menu_UNDO, self.doUndo)
		EVT_MENU(self, menu_REDO, self.doRedo)
		menuBar.Append(self.editMenu, "Edit")

		# Node menu
		self.nodeMenu = wxMenu()
		self.nodeMenu.Append(menu_SELECT_ALL_NODES,   "Select All\tCTRL-A")
		self.nodeMenu.Append(menu_DESELECT_ALL_NODES, "Deselect All")
		self.nodeMenu.AppendSeparator()
		#self.nodeMenu.Append(menu_CUT_NODE,           "Cut\tCTRL-X")
		#self.nodeMenu.Append(menu_COPY_NODE,          "Copy\tCTRL-C")
		#self.nodeMenu.Append(menu_PASTE_NODE,         "Paste\tCTRL-V")
		self.nodeMenu.Append(menu_DELETE_NODE,        "Delete\tCTRL-D")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_CONNECT,            "Connect")
		self.nodeMenu.Append(menu_DISCONNECT,         "Disconnect")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_SET_HOST,           "Host name(s)...")
		self.nodeMenu.Append(menu_SET_COUNT,          "Node Count...")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_SPLIT_NODES,        "Split")
		self.nodeMenu.Append(menu_MERGE_NODES,        "Merge")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_SERVER_OPTIONS,     "Server Options...")
		self.nodeMenu.Append(menu_SERVER_TILES,       "Server Tiles...")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_LAYOUT_NODES,       "Layout Nodes")
		EVT_MENU(self, menu_SELECT_ALL_NODES, self.doSelectAllNodes)
		EVT_MENU(self, menu_DESELECT_ALL_NODES, self.doDeselectAllNodes)
		EVT_MENU(self, menu_DELETE_NODE, self.doDeleteNodes)
		EVT_MENU(self, menu_CONNECT, self.doConnect)
		EVT_MENU(self, menu_DISCONNECT, self.doDisconnect)
		EVT_MENU(self, menu_SET_HOST, self.doSetHost)
		EVT_MENU(self, menu_SET_COUNT, self.doSetCount)
		EVT_MENU(self, menu_SPLIT_NODES, self.doSplitNodes)
		EVT_MENU(self, menu_MERGE_NODES, self.doMergeNodes)
		EVT_MENU(self, menu_SERVER_OPTIONS, self.doServerOptions)
		EVT_MENU(self, menu_SERVER_TILES, self.doServerTiles)
		EVT_MENU(self, menu_LAYOUT_NODES, self.doLayoutNodes)
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

		# Application menu
		self.systemMenu = wxMenu()
		self.systemMenu.Append(menu_APP_OPTIONS, "Options...")
		self.systemMenu.Append(menu_APP_RUN, "Run...")
		self.systemMenu.Append(menu_APP_STOP, "Stop...")
		EVT_MENU(self, menu_APP_OPTIONS, self.doAppOptions)
		menuBar.Append(self.systemMenu, "Application")

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

		# New Template button
#		templateLabel = wxStaticText(parent=self.topPanel, id=-1,
#									 label=" Template:")
#		toolSizer.Add(templateLabel, flag=wxALIGN_CENTRE)

		templateNames = [ "New Template" ] + templates.GetTemplateList()
		self.newTemplateChoice = wxChoice(parent=self.topPanel,
										  id=id_NewTemplate,
										  choices=templateNames)
		EVT_CHOICE(self.newTemplateChoice, id_NewTemplate, self.onNewTemplate)
		toolSizer.Add(self.newTemplateChoice, flag=wxEXPAND+wxALL, border=2)

		# Edit template settings button
		self.templateButton = wxButton(parent=self.topPanel,
									   id=id_TemplateOptions,
									   label="  Edit Template...  ")
		toolSizer.Add(self.templateButton, flag=wxEXPAND+wxALL, border=2)
		EVT_BUTTON(self.templateButton, id_TemplateOptions,
				   self.onTemplateEdit)

		# New app node button
		appChoices = ["New App Node(s)", "1 App node", "2 App nodes",
					  "3 App nodes", "4 App nodes", "N App nodes..."]
		self.newAppChoice = wxChoice(parent=self.topPanel, id=id_NewAppNode,
									  choices=appChoices)
		EVT_CHOICE(self.newAppChoice, id_NewAppNode, self.onNewAppNode)
		toolSizer.Add(self.newAppChoice, flag=wxEXPAND+wxALL, border=2)

		# New server node button
		serverChoices = ["New Server Node(s)", "1 Server node",
						 "2 Server nodes", "3 Server nodes", "4 Server nodes",
						 "N Server nodes..."]
		self.newServerChoice = wxChoice(parent=self.topPanel,
										id=id_NewServerNode,
										choices=serverChoices)
		EVT_CHOICE(self.newServerChoice, id_NewServerNode,
				   self.onNewServerNode)
		toolSizer.Add(self.newServerChoice, flag=wxEXPAND+wxALL, border=2)

		# New SPU button
		spuStrings = ["New SPU"] + SpuClasses
		self.newSpuChoice = wxChoice(parent=self.topPanel, id=id_NewSpu,
									 choices=spuStrings)
		EVT_CHOICE(self.newSpuChoice, id_NewSpu, self.onNewSpu)
		toolSizer.Add(self.newSpuChoice, flag=wxEXPAND+wxALL, border=2)

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
		topSizer.Add(toolSizer, option=0, flag=wxALL|wxALIGN_TOP, border=4)
		topSizer.Add(self.drawArea, 1, wxEXPAND)

		self.topPanel.SetAutoLayout(true)
		self.topPanel.SetSizer(topSizer)

		self.SetSizeHints(minW=500, minH=200)
		self.SetSize(wxSize(700, 400))

		# Create the hostnames dialog
		self.HostsDialog = hostdialog.HostDialog(parent=NULL, id=-1,
						title="Chromium Hosts",
						message="Specify host names for the selected nodes")


		self.dirty     = false
		self.fileName  = fileName
		self.mothership = crtypes.Mothership()
		self.undoStack = []
		self.redoStack = []
		self.LeftDown = 0
		self.DragStartX = 0
		self.DragStartY = 0
		self.SelectDeltaX = 0
		self.SelectDeltaY = 0

		if self.fileName != None:
			self.loadConfig()

		self.UpdateMenus()

	# ----------------------------------------------------------------------
	# Utility functions

	def SaveState(self):
		"""Save the current state to the undo stack.  This should be
		called immediately before any modifications to the project."""
		# make a deep copy of the current state (the mothership)
		state = copy.deepcopy(self.mothership)
		# put the copy on the undo stack
		self.undoStack.append(state)
		# erase the redo stack
		self.redoStack = []

	def Undo(self):
		"""Undo last change"""
		if len(self.undoStack) > 0:
			# put current state (mothership) onto the redo stack
			# XXX implement a max redo stack depth???
			self.redoStack.append(self.mothership)
			# get state from top of stack
			state = self.undoStack[-1]
			# replace current state with top of stack
			self.mothership = state
			# pop the undo stack
			self.undoStack.remove(state)
			
	def Redo(self):
		"""Redo last change"""
		if len(self.redoStack) > 0:
			# put current state (mothership) onto the undo stack
			# XXX implement a max undo stack depth???
			self.undoStack.append(self.mothership)
			# get top of redo stack
			state = self.redoStack[-1]
			# replace current state with top of stack
			self.mothership = state
			# pop the redo stack
			self.redoStack.remove(state)

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


    #----------------------------------------------------------------------
	# Event handlers / callbacks

	def onPaintEvent(self, event):
		"""Drawing area repaint callback"""
		dc = wxPaintDC(self.drawArea)
		self.drawArea.PrepareDC(dc)  # only for scrolled windows
		dc.BeginDrawing()

		# temporary
		#b = wxBrush(wxColor(120, 180, 220))
		#dc.SetBrush(b)
		#dc.DrawRectangle(50, 60, 350, 180)
		#dc.DrawText("Tilesort assembly", 55, 65)

		# draw the nodes
		for node in self.mothership.Nodes():
			if node.IsSelected():
				node.Draw(dc, self.SelectDeltaX, self.SelectDeltaY)
			else:
				node.Draw(dc)

		# draw the wires between the nodes
		pen = wxPen(wxColor(0, 0, 250))
		pen.SetWidth(2)
		dc.SetPen(pen)
		for node in self.mothership.Nodes():
			servers = node.GetServers()
			for serv in servers:
				p = node.GetOutputPlugPos()
				q = serv.GetInputPlugPos()
				dc.DrawLine( p[0], p[1], q[0], q[1] )
				# See if we need to draw triple lines for N-hosts
				dy1 = (node.GetCount() > 1) * 4
				dy2 = (serv.GetCount() > 1) * 4
				if dy1 != 0 or dy2 != 0:
					dc.DrawLine(p[0], p[1] + dy1, q[0], q[1] + dy2)
					dc.DrawLine(p[0], p[1] - dy1, q[0], q[1] - dy2)
		dc.EndDrawing()

	# called when New App Node button is pressed
	def onNewAppNode(self, event):
		self.SaveState()
		self.mothership.DeselectAllNodes()
		xstart = random.randrange(10, 50, 5)
		ystart = random.randrange(50, 100, 5)
		n = self.newAppChoice.GetSelection()
		if n < 5:
			for i in range(0, n):
				node = crutils.NewApplicationNode()
				node.SetPosition(xstart, ystart + i * 65)
				node.Select()
				self.mothership.AddNode(node)
		else:
			dialog = intdialog.IntDialog(parent=NULL, id=-1,
							   title="Create Application Nodes",
							   labels=["Number of Application nodes:"],
							   defaultValues=[1], minValue=1, maxValue=100)
			if dialog.ShowModal() == wxID_OK:
				n = dialog.GetValues()[0]
				if n > 0:
					node = crutils.NewApplicationNode(n)
					node.SetPosition(xstart, ystart)
					#node.SetCount(n)
					node.Select()
					self.mothership.AddNode(node)
		#endif
		self.newAppChoice.SetSelection(0)
		self.drawArea.Refresh()
		self.UpdateMenus()

	# called when New Server Node button is pressed
	def onNewServerNode(self, event):
		self.SaveState()
		self.mothership.DeselectAllNodes()
		xstart = random.randrange(250, 300, 5)
		ystart = random.randrange(50, 100, 5)
		n = self.newServerChoice.GetSelection()
		if n < 5:
			for i in range(0, n):
				node = crutils.NewNetworkNode()
				node.SetPosition(xstart, ystart + i * 65)
				node.Select()
				self.mothership.AddNode(node)
		else:
			dialog = intdialog.IntDialog(parent=NULL, id=-1,
							   title="Create Server Nodes",
							   labels=["Number of Server nodes:"],
							   defaultValues=[1], minValue=1, maxValue=100)
			if dialog.ShowModal() == wxID_OK:
				n = dialog.GetValues()[0]
				if n > 0:
					node = crutils.NewNetworkNode(n)
					node.SetPosition(xstart, ystart)
					#node.SetCount(n)
					node.Select()
					self.mothership.AddNode(node)
		#endif
				
		self.newServerChoice.SetSelection(0)
		self.drawArea.Refresh()
		self.UpdateMenus()

	def onNewSpu(self, event):
		"""New SPU button callback"""
		# add a new SPU to all selected nodes
		i = self.newSpuChoice.GetSelection()
		if i <= 0:
			return # didn't really select an SPU class
		self.SaveState()
		i -= 1
		for node in self.mothership.SelectedNodes():
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
			if pos >= 0 and crutils.SPUIsTerminal(SpuClasses[i]):
				self.Notify("You can't insert a %s SPU before a %s SPU." %
							(SpuClasses[i], node.GetSPU(pos).Name()))
				break
			# OK, we're all set, add the SPU
			s = crutils.NewSPU( SpuClasses[i] )
			node.AddSPU(s, pos)
		self.drawArea.Refresh()
		self.newSpuChoice.SetSelection(0)

	def onNewTemplate(self, event):
		"""New Template button callback"""
		self.SaveState()
		t = self.newTemplateChoice.GetSelection()
		if t > 0:
			templateName = templates.GetTemplateList()[t - 1]
			assert templateName != ""
			templates.CreateTemplate(templateName, self, self.mothership)
#		if t == 1:
#			templates.CreateTilesort(self, self.mothership)
#		elif t == 2:
#			templates.CreateSortlast(self, self.mothership)
#		elif t == 3:
#			templates.CreateNClientTilesort(self, self.mothership)
#		elif t == 4:
#			templates.CreateBinarySwap(self, self.mothership)
		self.newTemplateChoice.SetSelection(0)
		self.drawArea.Refresh()
		self.UpdateMenus()

	def onTemplateEdit(self, event):
		templateName = self.mothership.GetTemplateType()
		if templateName != "":
			templates.EditTemplate(templateName, self, self.mothership)
		else:
			self.Notify("This configuration doesn't match any template.")
		self.drawArea.Refresh()
		self.UpdateMenus()

	# Called when the left mouse button is pressed or released.
	def onMouseEvent(self, event):
		(x,y) = self.drawArea.CalcUnscrolledPosition(event.GetX(), event.GetY())
		# First, determine if we're clicking on an object
		# iterate backward through the object list so we get the topmost one
		hitNode = 0
		hitSPU = 0
		for i in range(len(self.mothership.Nodes()) - 1, -1, -1):
			node = self.mothership.Nodes()[i]
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
					self.UpdateSelection(self.mothership.Nodes(), hitNode, self.SELECT_EXTEND)
				else:
					self.UpdateSelection(hitNode.SPUChain(), 0, self.DESELECT_ALL)
					self.UpdateSelection(self.mothership.Nodes(), hitNode, mode)
			elif event.ControlDown() or event.ShiftDown():
				self.LeftDown = 0
			else: #if not 
				# didn't hit an SPU or a node
				for node in self.mothership.Nodes():
					self.UpdateSelection(node.SPUChain(), 0, self.DESELECT_ALL)
				self.UpdateSelection(self.mothership.Nodes(), 0, self.DESELECT_ALL)
		else:
			# mouse up
			if self.SelectDeltaX != 0 and self.SelectDeltaY != 0:
				self.SaveState()
			for node in self.mothership.SelectedNodes():
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
			for node in self.mothership.SelectedNodes():
				anySelected = 1
				break
			if anySelected:
				self.drawArea.Refresh()

	# ----------------------------------------------------------------------
	# File menu callbacks

	def doNew(self, event):
		"""File / New callback"""
		global _docList
		newFrame = MainFrame(None, -1, TitleString)
		newFrame.Show(TRUE)
		_docList.append(newFrame)


	def doOpen(self, event):
		"""File / Open callback"""
		global _docList

		curDir = os.getcwd()
		fileName = wxFileSelector("Open File",
								  default_extension="conf",
								  wildcard=WildcardPattern,
								  flags = wxOPEN | wxFILE_MUST_EXIST)
		if fileName == "":
			return
		fileName = os.path.join(os.getcwd(), fileName)
		os.chdir(curDir)

		if (self.fileName == None) and (len(self.mothership.Nodes()) == 0):
			# Load contents into current (empty) document.
			self.fileName = fileName
			title = TitleString + ": " + os.path.basename(fileName)
			self.SetTitle(title)
			self.loadConfig()
		else:
			# Open a new frame for this document.
			title = TitleString + ": " + os.path.basename(fileName)
			newFrame = MainFrame(None, -1, title, fileName=fileName)
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
			self.saveConfig()
		else:
			self.doSaveAs(event)

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

		title = TitleString + ": " + os.path.basename(fileName)
		self.SetTitle(title)

		self.fileName = fileName
		self.saveConfig()


	def doRevert(self, event):
		""" Respond to the "Revert" menu command.
		"""
		if not self.dirty:
			return

		if wxMessageBox("Discard changes made to this document?", "Confirm",
				style = wxOK | wxCANCEL | wxICON_QUESTION,
				parent=self) == wxCANCEL:
			return
		self.loadConfig()


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
	# Edit menu callbacks

	def doUndo(self, event):
		"""Edit / Undo callback"""
		self.Undo()
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doRedo(self, event):
		"""Edit / Redo callback"""
		self.Redo()
		self.drawArea.Refresh()
		self.UpdateMenus()

	# ----------------------------------------------------------------------
	# Node menu callbacks

	def doSelectAllNodes(self, event):
		"""Node / Select All callback"""
		self.mothership.SelectAllNodes()
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doDeselectAllNodes(self, event):
		"""Node / Deselect All callback"""
		self.mothership.DeselectAllNodes()
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doDeleteNodes(self, event):
		"""Node / Delete Nodes callback"""
		# Ugh, I can't find a Python counterpart to the C++ STL's remove_if()
		# function.
		# Have to make a temporary list of the objects to delete so we don't
		# screw-up the iteration as we remove things
		self.SaveState()
		deleteList = []
		for node in self.mothership.SelectedNodes():
			deleteList.append(node)
		# loop over nodes again to remove server connections
		for node in self.mothership.Nodes():
			for server in node.GetServers():
				if server.IsSelected():
					node.LastSPU().RemoveServer(server)
		# now delete the objects in the deleteList
		for node in deleteList:
			 self.mothership.Nodes().remove(node)
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doConnect(self, event):
		"""Node / Connect callback"""
		# Make list of packer(app and net) nodes and server nodes
		self.SaveState()
		netPackerNodes = []
		appPackerNodes = []
		serverNodes = []
		for node in self.mothership.SelectedNodes():
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
				if leftMost.LastSPU().MaxServers() > 1:
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
		self.SaveState()
		for node in self.mothership.SelectedNodes():
			#servers = node.GetServers()
			## make list of servers to remove
			#removeList = []
			#for s in servers:
			#	if s.IsSelected():
			#		removeList.append(s)
			## now remove them
			#for s in removeList:
			#	node.LastSPU().RemoveServer(s)
			# [:] syntax makes a copy of the list to prevent iteration problems
			for s in node.GetServers()[:]:
				if s.IsSelected():
					node.LastSPU().RemoveServer(s)
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doLayoutNodes(self, event):
		"""Node / Layout Nodes callback"""
		self.SaveState()
		self.mothership.LayoutNodes()
		self.drawArea.Refresh()

	def doSetHost(self, event):
		"""Node / Set Host callback"""
		assert len(self.mothership.SelectedNodes()) > 0
		# Load dialog values
		names = []
		count = 0
		for node in self.mothership.SelectedNodes():
			names += node.GetHosts()
			count += node.GetCount()
		self.HostsDialog.SetHosts(names)
		self.HostsDialog.SetCount(count)
		# XXX if more than one node is selected, use first node's pattern???
		firstSelected = self.mothership.SelectedNodes()[0]
		self.HostsDialog.SetHostPattern(firstSelected.GetHostNamePattern())
		# OK, show the dialog
		if self.HostsDialog.ShowModal() == wxID_OK:
			firstSelected.SetHostNamePattern(self.HostsDialog.GetHostPattern())
			newHosts = self.HostsDialog.GetHosts()
			# duplicate the last name until we have [count] names
			while len(newHosts) < count:
				newHosts.append(newHosts[-1])
			assert len(newHosts) >= count
			# this is a little tricky
			pos = 0
			for node in self.mothership.SelectedNodes():
				count = node.GetCount()
				node.SetHosts( newHosts[pos : pos + count] )
				pos += count
			self.drawArea.Refresh()
		return

	def doSetCount(self, event):
		"""Node / Set Count callback"""
		assert self.mothership.NumSelectedNodes() > 0
		n = self.mothership.SelectedNodes()[0].GetCount()
		dialog = intdialog.IntDialog(parent=NULL, id=-1,
						   title="Set Node Count",
						   labels=["Number of nodes:"],
						   defaultValues=[n], minValue=1, maxValue=100)
		if dialog.ShowModal() == wxID_OK:
			n = dialog.GetValues()[0]
			if n > 0:
				for node in self.mothership.SelectedNodes():
					node.SetCount(n)
		dialog.Destroy()
		self.drawArea.Refresh()

	def doSplitNodes(self, event):
		"""Node / Split callback"""
		for node in self.mothership.SelectedNodes():
			if node.GetCount() > 1:
				crutils.SplitNode(node, self.mothership)
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doMergeNodes(self, event):
		"""Node / Merge callback"""
		nodes = self.mothership.SelectedNodes()
		if not crutils.MergeNodes(nodes, self.mothership):
			self.Notify("The selected nodes are too dissimilar to be merged.")
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doServerOptions(self, event):
		"""Node / Server Options callback"""
		dialog = spudialog.SPUDialog(parent=self, id=-1,
									 title="Server Options",
									 options=self.mothership.ServerOptions)
		dialog.Centre()
		dialog.SetValues(self.mothership.GetServerOptions())
		if dialog.ShowModal() == wxID_OK:
			self.mothership.SetServerOptions(dialog.GetValues())

	def doServerTiles(self, event):
		"""Node / Server Tiles callback"""
		# find first server
		server = 0
		for node in self.mothership.SelectedNodes():
			if node.IsServer():
				server = node
				break
		assert server
		dialog = tiledialog.TileDialog(parent=self, id=-1,
									   numLists = server.GetCount(),
									   title="Server Tiles")
		dialog.Centre()
		# show the dialog
		tileLists = []
		for i in range(server.GetCount()):
			tileLists.append( server.GetTiles(i) )
		dialog.SetTileLists( tileLists )
		if dialog.ShowModal() == wxID_OK:
			tileLists = dialog.GetTileLists()
			for i in range(server.GetCount()):
				server.SetTiles( tileLists[i], i )


	# ----------------------------------------------------------------------
	# SPU menu callbacks

	def doSelectAllSPUs(self, event):
		"""SPU / Select All callback"""
		for node in self.mothership.SelectedNodes():
			for spu in node.SPUChain():
				spu.Select()
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doDeselectAllSPUs(self, event):
		"""SPU / Deselect All callback"""
		for node in self.mothership.SelectedNodes():
			for spu in node.SPUChain():
				spu.Deselect()
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doDeleteSPU(self, event):
		"""Node / Delete SPU callback"""
		# loop over all nodes, selected or not
		for node in self.mothership.Nodes():
			# [:] syntax makes a copy of the list to prevent iteration problems
			for spu in node.SPUChain()[:]:
				if spu.IsSelected():
					node.RemoveSPU(spu)
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doSpuOptions(self, event):
		"""SPU / SPU Options callback"""
		# find first selected SPU
		spuList = self.mothership.GetSelectedSPUs()
		if len(spuList) > 0:
			name = spuList[0].Name()
			if name in SPUInfo.keys():
				(params, opts) = SPUInfo[name]
				# create the dialog
				dialog = spudialog.SPUDialog(parent=self, id=-1,
											 title=name + " SPU Options",
											 options = opts)
				dialog.Centre()
				# set the dialog widget values
				dialog.SetValues(spuList[0].GetOptions())
				# wait for OK or cancel
				if dialog.ShowModal() == wxID_OK:
					# save the new values/options
					spuList[0].SetOptions(dialog.GetValues())
				else:
					# user cancelled, do nothing, new values are ignored
					pass
			else:
				print "Invalid SPU name: %s !!!" % name
		return
		
	# ----------------------------------------------------------------------
	# System menu callbacks
	
	def doAppOptions(self, event):
		"""Application / Options callback"""
		dialog = spudialog.SPUDialog(parent=self, id=-1,
									 title="Application Options",
									 options=self.mothership.GlobalOptions)
		dialog.Centre()
		dialog.SetValues(self.mothership.GetGlobalOptions())
		if dialog.ShowModal() == wxID_OK:
			self.mothership.SetGlobalOptions(dialog.GetValues())


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
		# Node menu
		if self.mothership.NumSelectedNodes() > 0:
			self.nodeMenu.Enable(menu_DELETE_NODE, 1)
			self.nodeMenu.Enable(menu_CONNECT, 1)
			self.nodeMenu.Enable(menu_DISCONNECT, 1)
			self.nodeMenu.Enable(menu_SET_HOST, 1)
			self.nodeMenu.Enable(menu_SET_COUNT, 1)
			self.nodeMenu.Enable(menu_SPLIT_NODES, 1)
			if self.mothership.NumSelectedNodes() > 1:
				self.nodeMenu.Enable(menu_MERGE_NODES, 1)
			else:
				self.nodeMenu.Enable(menu_MERGE_NODES, 0)
			self.newSpuChoice.Enable(1)
		else:
			self.nodeMenu.Enable(menu_DELETE_NODE, 0)
			self.nodeMenu.Enable(menu_CONNECT, 0)
			self.nodeMenu.Enable(menu_DISCONNECT, 0)
			self.nodeMenu.Enable(menu_SET_HOST, 0)
			self.nodeMenu.Enable(menu_SET_COUNT, 0)
			self.nodeMenu.Enable(menu_SPLIT_NODES, 0)
			self.nodeMenu.Enable(menu_MERGE_NODES, 0)
			self.newSpuChoice.Enable(0)
		# Node menu / servers
		if self.mothership.NumSelectedServers() > 0:
			self.nodeMenu.Enable(menu_SERVER_OPTIONS, 1)
			self.nodeMenu.Enable(menu_SERVER_TILES, 1)
		else:
			self.nodeMenu.Enable(menu_SERVER_OPTIONS, 0)
			self.nodeMenu.Enable(menu_SERVER_TILES, 0)
		if len(self.mothership.Nodes()) > 0:
			self.nodeMenu.Enable(menu_SELECT_ALL_NODES, 1)
			self.nodeMenu.Enable(menu_DESELECT_ALL_NODES, 1)
			self.nodeMenu.Enable(menu_LAYOUT_NODES, 1)
			self.spuMenu.Enable(menu_SELECT_ALL_SPUS, 1)
		else:
			self.nodeMenu.Enable(menu_SELECT_ALL_NODES, 0)
			self.nodeMenu.Enable(menu_DESELECT_ALL_NODES, 0)
			self.nodeMenu.Enable(menu_LAYOUT_NODES, 0)
			self.spuMenu.Enable(menu_SELECT_ALL_SPUS, 0)
		# SPU menu
		if self.mothership.NumSelectedSPUs() > 0:
			self.spuMenu.Enable(menu_DESELECT_ALL_SPUS, 1)
			self.spuMenu.Enable(menu_DELETE_SPU, 1)
			self.spuMenu.Enable(menu_SPU_OPTIONS, 1)
		else:
			self.spuMenu.Enable(menu_DESELECT_ALL_SPUS, 0)
			self.spuMenu.Enable(menu_DELETE_SPU, 0)
			self.spuMenu.Enable(menu_SPU_OPTIONS, 0)
		# Edit menu
		if len(self.undoStack) > 0:
			self.editMenu.Enable(menu_UNDO, 1)
		else:
			self.editMenu.Enable(menu_UNDO, 0)
		if len(self.redoStack) > 0:
			self.editMenu.Enable(menu_REDO, 1)
		else:
			self.editMenu.Enable(menu_REDO, 0)
		# Template options button
		self.templateButton.Enable(1) # XXX fix sometime
#		type = self.mothership.GetTemplateType()
#		if type == "":
#			self.templateButton.Enable(0)
#		else:
#			self.templateButton.Enable(1)

	# Display a dialog with a message and OK button.
	def Notify(self, msg):
		dialog = wxMessageDialog(parent=self, message=msg,
								 caption="Hey!", style=wxOK)
		dialog.ShowModal()


	#----------------------------------------------------------------------
	# File I/O

	def loadConfig(self):
		"""Load a graph from a file"""
		f = open(self.fileName, "r")
		if f:
			# read first line to check for template
			l = f.readline()
			if not l:
				return
			v = re.search("^TEMPLATE = \"?([^\"]*)\"?", l)
			if v:
				template = v.group(1)
				if not templates.ReadTemplate(template, self.mothership, f):
					configio.ReadConfig(self.mothership, f)
			f.close()
			self.dirty = false
		else:
			self.Notify("Problem opening " + self.fileName)
		self.drawArea.Refresh()


	def saveConfig(self):
		"""Save the Chromium configuration to a file."""
		print "Saving graph!"
		f = open(self.fileName, "w")
		if f:
			template = self.mothership.GetTemplateType()
			if (template != "" and
				templates.ValidateTemplate(template, self.mothership)):
				# write as templatized config
				templates.WriteTemplate(template, self.mothership, f)
			else:
				# write as generic config
				configio.WriteConfig(self.mothership, f)
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

			self.saveConfig()
			return true
		elif response == wxNO:
			return true # User doesn't want changes saved.
		elif response == wxCANCEL:
			return false # User cancelled.


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
			frame = MainFrame(None, -1, TitleString)
			frame.Centre()
			frame.Show(TRUE)
			_docList.append(frame)
		else:
			# Load the file(s) specified on the command line.
			for arg in sys.argv[1:]:
				fileName = os.path.join(os.getcwd(), arg)
				if os.path.isfile(fileName):
					title = TitleString + ": " + os.path.basename(fileName)
					frame = MainFrame(None, -1, title, fileName=fileName)
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
	# XXX if we crash upon startup, try commenting-out this next line:
	sys.stderr = ExceptionHandler()

	# Set the default site file
	#crutils.SetDefaultSiteFile("tungsten.crsite")

	# Scan for available SPU classes
	global SpuClasses
	SpuClasses = crutils.FindSPUNames()
	print "Found SPU classes: %s" % str(SpuClasses)

	# Get the params and options for all SPU classes
	global SPUInfo
	SPUInfo = {}
	for spu in SpuClasses:
		print "getting options for %s SPU" % spu
		SPUInfo[spu] = crutils.GetSPUOptions(spu)

	# Create and start the application.
	_app = ConfigApp(0)
	_app.MainLoop()


if __name__ == "__main__":
	main()


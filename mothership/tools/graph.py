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


import string, os.path
from wxPython.wx import *

import traceback, types

#----------------------------------------------------------------------------
#                            System Constants
#----------------------------------------------------------------------------

# Our menu item IDs:

menu_UNDO          = 10001 # Edit menu items.
menu_SELECT_ALL    = 10002
menu_DELETE        = 10003
menu_CONNECT       = 10004
menu_DISCONNECT    = 10005
menu_SET_HOST      = 10006
menu_SPU_OPTIONS   = 10007
menu_HELP          = 10008
menu_ABOUT         = 10009

# Widget IDs
id_NewServerNode  = 3000
id_NewAppNode     = 3001
id_NewSpu         = 3002
id_NewTemplate    = 3003

# Size of the drawing page, in pixels.
PAGE_WIDTH  = 1000
PAGE_HEIGHT = 1000

SpuClasses = [ "New SPU", "Pack", "Render", "Readback", "Tilesort", "Wet", "Hiddenline" ]

# How many servers can this SPU connect to
SpuMaxServers = { "Pack" : 1,  "Tilesort" : 1000 }

# SPUs which must be at the end of a chain, other than packers
TerminalSPUs = [ "Render" ]

Templates = [ "New Template", "Tilesort", "Sort-last" ]


AppNodeBrush = wxBrush(wxColor(55, 160, 55))
ServerNodeBrush = wxBrush(wxColor(210, 105, 135))
BackgroundColor = wxColor(90, 150, 190)

#----------------------------------------------------------------------------
# Utility functions

class HostGenerator:
	"""Utility class for generating a sequence of numbered hostnames."""
	_letterPattern = "[a-zA-Z0-9_\.\-]*"
	_hashPattern = "#*"
	_fullPattern = "^" + _letterPattern + _hashPattern + _letterPattern + "$"

	def ValidateFormatString(format):
		if re.match(_fullPattern, format):
			return 1
		else:
			return 0

	def MakeHostname(format, number):
		# find the hash characters first
		p = re.match(_hashPattern, format)
		len = len(p.string)
		print "len = %s" % len
		return "host05"




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
		self.editMenu.Append(menu_UNDO,          "Undo\tCTRL-Z")
		self.editMenu.AppendSeparator()
		self.editMenu.Append(menu_DELETE,        "Delete\tCTRL-D")
		self.editMenu.Append(menu_SELECT_ALL,    "Select All\tCTRL-A")
		self.editMenu.AppendSeparator()
		self.editMenu.Append(menu_CONNECT,       "Connect Nodes")
		self.editMenu.Append(menu_DISCONNECT,    "Disconnect Nodes")
		self.editMenu.Append(menu_SET_HOST,      "Set Host...")
		self.editMenu.Append(menu_SPU_OPTIONS,   "SPU Options...")
		EVT_MENU(self, menu_DELETE, self.doDelete)
		EVT_MENU(self, menu_SELECT_ALL, self.doSelectAll)
		EVT_MENU(self, menu_CONNECT, self.doConnect)
		EVT_MENU(self, menu_DISCONNECT, self.doDisconnect)
		EVT_MENU(self, menu_SET_HOST, self.doSetHost)
		EVT_MENU(self, menu_SPU_OPTIONS, self.doSpuOptions)
		menuBar.Append(self.editMenu, "Edit")

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
		toolSizer.Add(self.newAppChoice, flag=wxEXPAND+wxALL, border=4)

		# New server node button
		serverChoices = ["New Server Node(s)", "1 Server node", "2 Server nodes",
						 "3 Server nodes", "4 Server nodes"]
		self.newServerChoice = wxChoice(parent=self.topPanel, id=id_NewServerNode,
									  choices=serverChoices)
		EVT_CHOICE(self.newServerChoice, id_NewServerNode, self.onNewServerNode)
		toolSizer.Add(self.newServerChoice, flag=wxEXPAND+wxALL, border=4)

		# New SPU button
		self.newSpuChoice = wxChoice(parent=self.topPanel, id=id_NewSpu,
									 choices=SpuClasses)
		EVT_CHOICE(self.newSpuChoice, id_NewSpu, self.onNewSpu)
		toolSizer.Add(self.newSpuChoice, flag=wxEXPAND+wxALL, border=4)

		# New Template button
		self.newTemplateChoice = wxChoice(parent=self.topPanel, id=id_NewTemplate,
									 choices=Templates)
		EVT_CHOICE(self.newTemplateChoice, id_NewTemplate, self.onNewTemplate)
		toolSizer.Add(self.newTemplateChoice, flag=wxEXPAND+wxALL, border=4)

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


	# ----------------------------------------------------------------------
	# Node functions

	def SelectAll(self):
		for node in self.Nodes:
			node.Select()

	def DeselectAll(self):
		for node in self.Nodes:
			node.Deselect()

	def AddNode(self, node):
		"""Add a new node to the system"""
		self.Nodes.append(node)

	def RemoveNode(self, node):
		"""Remove a node from the system"""
		if node in self.Nodes:
			self.Nodes.remove(node)


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
		dc.SetPen(pen);
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
		n = self.newAppChoice.GetSelection()
		for i in range(0, n):
			node = ApplicationNode()
			node.SetPosition(x=10, y=50+i*65)
			node.Select()
			self.AddNode(node)
		self.newAppChoice.SetSelection(0)
		self.drawArea.Refresh()

	# called when New Server Node button is pressed
	def onNewServerNode(self, event):
		self.DeselectAll()
		n = self.newServerChoice.GetSelection()
		for i in range(0, n):
			node = NetworkNode()
			node.SetPosition(x=250, y=50+i*65)
			node.Select()
			self.AddNode(node)
		self.newServerChoice.SetSelection(0)
		self.drawArea.Refresh()

	def onNewSpu(self, event):
		"""New SPU button callback"""
		# add a new SPU to all selected nodes
		i = self.newSpuChoice.GetSelection()
		if i > 0:
			for node in self.Nodes:
				if node.IsSelected():
					if node.NumSPUs() > 0 and (node.LastSPU().IsPacker() or node.LastSPU().IsTerminal()):
						self.Notify("You can't chain a %s SPU after a %s SPU." % (SpuClasses[i], node.LastSPU().Name()))
						break
					else:
						s = SpuObject( SpuClasses[i] )
						node.AddSPU(s)
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
		return

	# Called when the left mouse button is pressed or released.
	def onMouseEvent(self, event):
		(x,y) = self.drawArea.CalcUnscrolledPosition(event.GetX(), event.GetY())
		# First, determine if we're clicking on an object
		# iterate backward through the object list so we get the topmost one
		hitNode = 0
		for i in range(len(self.Nodes) - 1, -1, -1):
			node = self.Nodes[i]
			p = node.PickTest(x,y)
			if p >= 1:
				hitNode = node
				if p > 1 and event.LeftDown():
					# clicked on the p-2 SPU in the node
					if node.IsSelectedSPU(p-2):
						node.DeselectSPU(p - 2)
					else:
						node.SelectSPU(p - 2)
				break
			# endif
		# endfor

		self.LeftDown = event.LeftDown()

		# Now handle selection/deselection
		if self.LeftDown:
			# mouse down
			if hitNode:
				self.DragStartX = x
				self.DragStartY = y
				self.SelectDeltaX = 0
				self.SelectDeltaY = 0
				if event.ControlDown():
					# toggle selection status of one object
					if hitNode.IsSelected():
						hitNode.Deselect()
					else:
						hitNode.Select()
				elif not hitNode.IsSelected():
					if not event.ShiftDown():
						for node in self.Nodes:
							node.Deselect()
					hitNode.Select()
			else:
				# deselect all
				for node in self.Nodes:
					node.Deselect()
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

		self.drawArea.Refresh()

	# Called when the mouse moves.  We only really need to call this when a
	# mouse button is also pressed, but I don't see a way to specify that with
	# wxWindows as you can do with X.
	def onMouseMotion(self, event):
		if event.LeftIsDown():
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

	#----------------------------------------------------------------------
	# Menu callbacks

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
		fileName = wxFileSelector("Open File", default_extension="psk",
					  flags = wxOPEN | wxFILE_MUST_EXIST)
		if fileName == "": return
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
			if not self.askIfUserWantsToSave("closing"): return

		_docList.remove(self)
		self.Destroy()


	def doSave(self, event):
		"""File / Save callback"""
		if self.fileName != None:
			self.saveContents()


	def doSaveAs(self, event):
		"""File / Save As callback"""
		if self.fileName == None:
			default = ""
		else:
			default = self.fileName

		curDir = os.getcwd()
		fileName = wxFileSelector("Save File As", "Saving",
					  default_filename=default,
					  default_extension="psk",
					  wildcard="*.psk",
					  flags = wxSAVE | wxOVERWRITE_PROMPT)
		if fileName == "": return # User cancelled.
		fileName = os.path.join(os.getcwd(), fileName)
		os.chdir(curDir)

		title = os.path.basename(fileName)
		self.SetTitle(title)

		self.fileName = fileName
		self.saveContents()


	def doRevert(self, event):
		""" Respond to the "Revert" menu command.
		"""
		if not self.dirty: return

		if wxMessageBox("Discard changes made to this document?", "Confirm",
				style = wxOK | wxCANCEL | wxICON_QUESTION,
				parent=self) == wxCANCEL: return
		self.loadContents()


	def doExit(self, event):
		""" Respond to the "Quit" menu command.
		"""
		global _docList, _app
		for doc in _docList:
			if not doc.dirty: continue
			doc.Raise()
			if not doc.askIfUserWantsToSave("quitting"): return
			_docList.remove(doc)
			doc.Destroy()

		_app.ExitMainLoop()


	def doDelete(self, event):
		"""Edit / Delete callback"""
		# Ugh, I can't find a Python counterpart to the C++ STL's remove_if()
		# function.
		# Have to make a temporary list of the objects to delete so we don't
		# screw-up the iteration as we remove things
		deleteList = []
		for node in self.Nodes:
			if node.IsSelected():
				deleteList.append(node)
		# now delete the objects in the deleteList
		for node in deleteList:
			 self.Nodes.remove(node)
		self.drawArea.Refresh()

	def doSelectAll(self, event):
		"""Edit / Select All callback"""
		for node in self.Nodes:
			node.Select()
		self.drawArea.Refresh()

	def doConnect(self, event):
		"""Edit / Connect callback"""
		# First, count how many app nodes and network/server nodes we have
		numAppNodes = 0
		numServerNodes = 0
		for node in self.Nodes:
			if node.IsSelected():
				if node.IsServer():
					numServerNodes += 1
				elif node.IsAppNode() and node.HasAvailablePacker():
					numAppNodes += 1

		# Now, wire-up the connections
		if numAppNodes == 1 and numServerNodes > 0:
			# connect the app node to all server nodes (probably tilesort)
			for node in self.Nodes:
				if node.IsSelected() and node.IsAppNode() and node.HasAvailablePacker():
					# look for server nodes
					for n in self.Nodes:
						if n.IsSelected() and n.IsServer():
							node.LastSPU().AddServer(n)
					#endfor
			#endfor
		elif numAppNodes > 0 and numServerNodes == 1:
			# connect all app nodes to the server (probably sort-last)
			for node in self.Nodes:
				if node.IsSelected() and node.IsServer():
					# look for app nodes
					for n in self.Nodes:
						if n.IsSelected() and n.IsAppNode() and n.HasAvailablePacker():
							n.LastSPU().AddServer(node)
					#endfor
			#endfor
		else:
			# not sure what to do here yet
			print "doConnect() not fully implemented yet, sorry."
			
		self.drawArea.Refresh()

	def doDisconnect(self, event):
		"""Edit / Disconnect callback"""
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

	def doSetHost(self, event):
		"""Edit / Set Host callback"""
		dialog = wxTextEntryDialog(self, message="Enter the hostname for the selected nodes")
		dialog.SetTitle("Chromium host")
		if dialog.ShowModal() == wxID_OK:
			for node in self.Nodes:
				if node.IsSelected():
					node.SetHost(dialog.GetValue())
		dialog.Destroy()
		self.drawArea.Refresh()

	def doSpuOptions(self, event):
		"""Edit / SPU Options callback"""
		return
		
	def doShowIntro(self, event):
		"""Help / Introduction callback"""
		dialog = wxDialog(self, -1, "Introduction") # ,
				  #style=wxDIALOG_MODAL | wxSTAY_ON_TOP)
		dialog.SetBackgroundColour(wxWHITE)

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
		dialog = wxDialog(self, -1, "About") # ,
				  #style=wxDIALOG_MODAL | wxSTAY_ON_TOP)
		dialog.SetBackgroundColour(wxWHITE)

		panel = wxPanel(dialog, -1)
		panel.SetBackgroundColour(wxWHITE)

		panelSizer = wxBoxSizer(wxVERTICAL)

		boldFont = wxFont(panel.GetFont().GetPointSize(),
				  panel.GetFont().GetFamily(),
				  wxNORMAL, wxBOLD)

		logo = wxStaticBitmap(panel, -1, wxBitmap("images/logo.bmp",
							  wxBITMAP_TYPE_BMP))

		lab1 = wxStaticText(panel, -1, "not done yet")
		lab1.SetFont(wxFont(12, boldFont.GetFamily(), wxITALIC, wxBOLD))
		lab1.SetSize(lab1.GetBestSize())

		imageSizer = wxBoxSizer(wxHORIZONTAL)
		imageSizer.Add(logo, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 5)
		imageSizer.Add(lab1, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 5)

		btnOK = wxButton(panel, wxID_OK, "OK")

		panelSizer.Add(imageSizer, 0, wxALIGN_CENTRE)
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

	# Display a dialog with a message and OK button.
	def Notify(self, msg):
		dialog = wxMessageDialog(parent=self, message=msg,
								 caption="Hey!", style=wxOK)
		dialog.ShowModal()


	#----------------------------------------------------------------------
	# File I/O

	def loadContents(self):
		""" Load the contents of our document into memory.
		"""
		self.dirty = false

		self.drawArea.Refresh()


	def saveContents(self):
		""" Save the contents of our document to disk.
		"""
		self.dirty = false


	def askIfUserWantsToSave(self, action):
		""" Give the user the opportunity to save the current document.

			'action' is a string describing the action about to be taken.  If
			the user wants to save the document, it is saved immediately.  If
			the user cancels, we return false.
		"""
		if not self.dirty: return true # Nothing to do.

		response = wxMessageBox("Save changes before " + action + "?",
					"Confirm", wxYES_NO | wxCANCEL, self)

		if response == wxYES:
			if self.fileName == None:
				fileName = wxFileSelector("Save File As", "Saving",
						  default_extension="psk",
						  wildcard="*.psk",
						  flags = wxSAVE | wxOVERWRITE_PROMPT)
				if fileName == "": return false # User cancelled.
				self.fileName = fileName

			self.saveContents()
			return true
		elif response == wxNO:
			return true # User doesn't want changes saved.
		elif response == wxCANCEL:
			return false # User cancelled.


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

class SpuObject:
	def __init__(self, name):
		self.X = 0
		self.Y = 0
		self._Name = name
		self.Width = 0
		self.Height = 30
		self._IsSelected = 0
		self._Port = 7000
		self._Protocol = "tcpip"
		self._Servers = []

	def IsPacker(self):
		"""Return true if this SPU has a packer"""
		if self._Name in SpuMaxServers.keys():
			return 1
		else:
			return 0

	def IsTerminal(self):
		"""Return true if this SPU has to be the last in a chain (a terminal)
		"""
		return self._Name in TerminalSPUs

	def Select(self):
		self._IsSelected = 1

	def Deselect(self):
		self._IsSelected = 0

	def IsSelected(self):
		return self._IsSelected

	def CanAddServer(self):
		"""Test if a server can be added to this SPU.
		Return "OK" if so, else return reason why not.
		"""
		if not self.IsPacker():
			return "This SPU doesn't have a command packer"
		if len(self._Servers) >= SpuMaxServers[self._Name]:
			return "This SPU is limited to %d server(s)" % SpuMaxServers[self._Name]
		return "OK"

	def AddServer(self, serverNode, protocol='tcpip', port=7000):
		"""Add a server to this SPU.  The SPU must have a packer!"""
		if self.IsPacker() and len(self._Servers) < SpuMaxServers[self._Name]:
			self._Servers.append(serverNode)
			self._Protocol = protocol
			self._Port = port

	def RemoveServer(self, serverNode):
		if serverNode in self._Servers:
			self._Servers.remove(serverNode)

	def GetServers(self):
		"""Return the list of servers for this SPU.
		For a pack SPU the list will contain zero or one server.
		For a tilesort SPU the list will contain zero or more servers.
		Otherw SPU classes have no servers.
		"""
		return self._Servers

	def Name(self):
		return self._Name

	def SetPosition(self, x, y):
		self.X = x
		self.Y = y

	def GetPosition(self):
		return (self.X, self.Y)

	def GetWidth(self):
		return self.Width

	def GetHeight(self):
		return self.Height

	def PickTest(self, x, y):
		if x >= self.X and x < self.X + self.Width and y >= self.Y and y < self.Y + self.Height:
			return 1
		else:
			return 0

	def Draw(self, dc):
		dc.SetBrush(wxLIGHT_GREY_BRUSH)
		p = wxPen(wxColor(0,0,0), width=1, style=0)
		if self._IsSelected:
			p.SetWidth(3)
		dc.SetPen(p)
		# if width is zero, compute it now based on the text width
		if self.Width == 0:
			(self.Width, unused) = dc.GetTextExtent(self._Name)
			self.Width += 10
		# draw the SPU as rectangle with text label
		dc.DrawRectangle(self.X, self.Y, self.Width, self.Height)
		dc.DrawText(self._Name, self.X + 3, self.Y + 3)
		if self.IsPacker():
			dc.SetBrush(wxBLACK_BRUSH)
			dc.DrawRectangle(self.X + self.Width, self.Y + self.Height/2 - 4, 4, 8)

class Node:
	""" The graphical representation of a Cr node (app or network).
	This is the base class for the ServerNode and NetworkNode classes.
	"""

	def __init__(self, host="localhost", isServer = 0):
		self.X = 0
		self.Y = 0
		self.Width = 200
		self.Height = 60
		self.SpuChain = []
		self._IsServer = isServer
		self._IsSelected = 0
		self._InputPlugPos = (0,0)
		self._Host = host

	def IsAppNode(self):
		"""Return true if this is an app node, false otherwise."""
		return not self._IsServer

	def IsServer(self):
		"""Return true if this is a server, false otherwise."""
		return self._IsServer

	def HasAvailablePacker(self):
		"""Return true if we can connect a server to this node."""
		if len(self.SpuChain) >= 1 and self.LastSPU().CanAddServer() == "OK":
			return 1
		else:
			return 0

	def HasPacker(self):
		"""Return true if the last SPU has a packer."""
		if len(self.SpuChain) >= 1 and self.LastSPU().IsPacker():
			return 1
		else:
			return 0

	def SetHost(self, hostname):
		self._Host = hostname

	def Select(self):
		self._IsSelected = 1

	def Deselect(self):
		self._IsSelected = 0

	def IsSelected(self):
		return self._IsSelected

	def AddSPU(self, s):
		self.SpuChain.append(s)

	def NumSPUs(self):
		return len(self.SpuChain)

	def LastSPU(self):
		return self.SpuChain[-1]

	def GetServers(self):
		if self.NumSPUs() > 0:
			return self.LastSPU().GetServers()
		else:
			return []

	def SelectSPU(self, i):
		self.SpuChain[i].Select()

	def DeselectSPU(self, i):
		self.SpuChain[i].Deselect()

	def IsSelectedSPU(self, i):
		return self.SpuChain[i].IsSelected()

	def GetPosition(self):
		return (self.X, self.Y)

	def SetPosition(self, x, y):
		self.X = x;
		self.Y = y;

	# Return the (x,y) coordinate of the node's input socket
	def GetInputPlugPos(self):
		assert self._IsServer
		return self._InputPlugPos

	# Return the (x,y) coordinate of the node's output socket
	def GetOutputPlugPos(self):
		assert self.NumSPUs() > 0
		last = self.LastSPU()
		assert last.IsPacker()
		(x, y) = last.GetPosition()
		x += last.GetWidth()
		y += last.GetHeight() / 2
		return (x, y)

	def Draw(self, dc, dx=0, dy=0):
		# setup the brush and pen
		if self._IsServer:
			dc.SetBrush(ServerNodeBrush);
		else:
			dc.SetBrush(AppNodeBrush);
		p = wxPen(wxColor(0,0,0), width=1, style=0)
		if self._IsSelected:
			p.SetWidth(3)
		dc.SetPen(p)
		x = self.X + dx
		y = self.Y + dy
		# draw the node's box
		dc.DrawRectangle(x, y, self.Width, self.Height)
		if self._IsServer:
			dc.DrawText("Server node host=%s" % self._Host, x + 3, y + 3)
			# draw the unpacker plug
			px = x - 4
			py = y + self.Height / 2
			self._InputPlugPos = (px, py)
			dc.SetBrush(wxBLACK_BRUSH)
			dc.DrawRectangle(px, py - 4, 4, 8)
		else:
			dc.DrawText("App node host=%s" % self._Host, x + 3, y + 3)

		# draw the SPUs
		x = x + 5
		y = y + 20
		for spu in self.SpuChain:
			spu.SetPosition(x, y)
			spu.Draw(dc)
			x = x + spu.GetWidth() + 2


	def PickTest(self, x, y):
		# try the SPUs first
		i = 0
		for spu in self.SpuChain:
			if spu.PickTest(x,y):
				return 2 + i
			i = i + 1
		# now try the node itself
		if x >= self.X and x < self.X + self.Width and y >= self.Y and y < self.Y + self.Height:
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


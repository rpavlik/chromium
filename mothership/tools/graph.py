""" graph.py

    Tool for making arbitrary Chromium configuration graphs.

"""


import string, cPickle, os.path
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

menu_HELP          = 2001 # Help menu items.
menu_ABOUT         = 2002 # Help menu items.

# Widget IDs
id_MuralWidth  = 3000
id_MuralHeight = 3001
id_TileChoice  = 3002
id_TileWidth   = 3003
id_TileHeight  = 3004
id_NewServerNode  = 3008
id_NewAppNode     = 3009
id_NewSpu         = 3010

# Size of the drawing page, in pixels.
PAGE_WIDTH  = 1000
PAGE_HEIGHT = 1000

CommonTileSizes = [ [128, 128],
					[256, 256],
					[512, 512],
					[1024, 1024],
					[1280, 1024],
					[1600, 1200] ]

SpuClasses = [ "New SPU", "Pack", "Render", "Readback", "Tilesort", "Wet", "Hiddenline" ]

# SPUs that pack for a downstream server
PackingSPUs = [ "Pack", "Tilesort" ]

# SPUs which must be at the end of a chain, other than packers
TerminalSPUs = [ "Render" ]


AppNodeBrush = wxBrush(wxColor(55, 160, 55))
ServerNodeBrush = wxBrush(wxColor(210, 105, 135))
BackgroundColor = wxColor(90, 150, 190)

#----------------------------------------------------------------------------

class DrawingFrame(wxFrame):
	""" A frame showing the contents of a single document. """

	# ==========================================
	# == Initialisation and Window Management ==
	# ==========================================

	def __init__(self, parent, id, title, fileName=None):
		""" Standard constructor.

			'parent', 'id' and 'title' are all passed to the standard wxFrame
			constructor.  'fileName' is the name and path of a saved file to
			load into this frame, if any.
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
		EVT_MENU(self, menu_DELETE, self.doDelete)
		EVT_MENU(self, menu_SELECT_ALL, self.doSelectAll)
		EVT_MENU(self, menu_CONNECT, self.doConnect)
		EVT_MENU(self, menu_DISCONNECT, self.doDisconnect)
		EVT_MENU(self, menu_SET_HOST, self.doSetHost)
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
		appChoices = ["New App Node(s)", "1 App node", "2 App nodes", "3 App nodes", "4 App nodes"]
		self.newAppChoice = wxChoice(parent=self.topPanel, id=id_NewAppNode,
									  choices=appChoices)
		EVT_CHOICE(self.newAppChoice, id_NewAppNode, self.onNewAppNode)
		toolSizer.Add(self.newAppChoice, flag=wxEXPAND+wxALL, border=4)
		# New server node button
		serverChoices = ["New Server Node(s)", "1 Server node", "2 Server nodes", "3 Server nodes", "4 Server nodes"]
		self.newServerChoice = wxChoice(parent=self.topPanel, id=id_NewServerNode,
									  choices=serverChoices)
		EVT_CHOICE(self.newServerChoice, id_NewServerNode, self.onNewServerNode)
		toolSizer.Add(self.newServerChoice, flag=wxEXPAND+wxALL, border=4)
		# New SPU button
		self.newSpuChoice = wxChoice(parent=self.topPanel, id=id_NewSpu, choices=SpuClasses)
		EVT_CHOICE(self.newSpuChoice, id_NewSpu, self.onNewSpu)
		toolSizer.Add(self.newSpuChoice, flag=wxEXPAND+wxALL, border=4)
#		newSpuButton = wxButton(parent=self.topPanel, id=id_NewSpu, label="New SPU")
#		EVT_BUTTON(newSpuButton, id_NewSpu, self.onNewSpu)
#		toolSizer.Add(newSpuButton)

		# Setup the main drawing area.
		self.drawArea = wxScrolledWindow(self.topPanel, -1,
										 style=wxSUNKEN_BORDER)
		self.drawArea.EnableScrolling(true, true)
		self.drawArea.SetScrollbars(20, 20, PAGE_WIDTH / 20, PAGE_HEIGHT / 20)
#		self.drawArea = wxPanel(self.topPanel, id=-1, style=wxSUNKEN_BORDER)
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
		self.Objects = []
		self.Connections = []
		self.LeftDown = 0
		self.DragStartX = 0
		self.DragStartY = 0
		self.SelectDeltaX = 0
		self.SelectDeltaY = 0

		if self.fileName != None:
			self.loadContents()


	def SelectAll(self):
		for obj in self.Objects:
			obj.Select()

	def DeselectAll(self):
		for obj in self.Objects:
			obj.Deselect()

	def AddConnection(self, fromNode, toNode):
		self.Connections.append((fromNode, toNode))

	def RemoveConnections(self, targetNode):
		# XXX this isn't 100% correct
		for c in self.Connections:
			(fromNode, toNode) = c
			if fromNode == targetNode or toNode == targetNode:
				self.Connections.remove(c)


	# ============================
	# == Event Handling Methods ==
	# ============================

	def onPaintEvent(self, event):
		""" Respond to a request to redraw the contents of our drawing panel.
		"""
		dc = wxPaintDC(self.drawArea)
		self.drawArea.PrepareDC(dc)  # only for scrolled windows
		dc.BeginDrawing()

		# draw the nodes
		for obj in self.Objects:
			if obj.IsSelected():
				obj.Draw(dc, self.SelectDeltaX, self.SelectDeltaY)
			else:
				obj.Draw(dc)

		# draw the wires between the nodes
		pen = wxPen(wxColor(0, 0, 250))
		pen.SetWidth(2)
		dc.SetPen(pen);
		for conn in self.Connections:
			(fromNode, toNode) = conn
			p = fromNode.GetOutputPlugPos()
			q = toNode.GetInputPlugPos()
			dc.DrawLine( p[0], p[1], q[0], q[1])

		dc.EndDrawing()

	# called when New App Node button is pressed
	def onNewAppNode(self, event):
		self.DeselectAll()
		n = self.newAppChoice.GetSelection()
		for i in range(0, n):
			obj = NodeObject(x=10, y=50+i*65, isServer=0)
			obj.Select()
			self.Objects.append(obj)
		self.newAppChoice.SetSelection(0)
		self.drawArea.Refresh()

	# called when New Server Node button is pressed
	def onNewServerNode(self, event):
		self.DeselectAll()
		n = self.newServerChoice.GetSelection()
		for i in range(0, n):
			obj = NodeObject(x=250, y=50+i*65, isServer=1)
			obj.Select()
			self.Objects.append(obj)
		self.newServerChoice.SetSelection(0)
		self.drawArea.Refresh()

	# called when New SPU button is pressed
	def onNewSpu(self, event):
		# add a new SPU to all selected nodes
		i = self.newSpuChoice.GetSelection()
		if i > 0:
			for obj in self.Objects:
				if obj.IsSelected():
					if obj.NumSPUs() > 0 and (obj.LastSPU().IsPacker() or obj.LastSPU().IsTerminal()):
						self.Notify("You can't chain a %s SPU after a %s SPU." % (SpuClasses[i], obj.LastSPU().Name()))
						break
					else:
						p = (SpuClasses[i] in PackingSPUs)
						s = SpuObject( SpuClasses[i], packer=p )
						obj.AddSPU(s)
			self.drawArea.Refresh()
			self.newSpuChoice.SetSelection(0)

	# Called when the left mouse button is pressed or released.
	def onMouseEvent(self, event):
		(x,y) = self.drawArea.CalcUnscrolledPosition(event.GetX(), event.GetY())
		# First, determine if we're clicking on an object
		# iterate backward through the object list so we get the topmost one
		hitObj = 0
		for i in range(len(self.Objects) - 1, -1, -1):
			obj = self.Objects[i]
			p = obj.PickTest(x,y)
			if p >= 1:
				hitObj = obj
				if p > 1 and event.LeftDown():
					# clicked on the p-2 SPU in the node
					if obj.IsSelectedSPU(p-2):
						obj.DeselectSPU(p - 2)
					else:
						obj.SelectSPU(p - 2)
				break
			# endif
		# endfor

		self.LeftDown = event.LeftDown()

		# Now handle selection/deselection
		if self.LeftDown:
			# mouse down
			if hitObj:
				self.DragStartX = x
				self.DragStartY = y
				self.SelectDeltaX = 0
				self.SelectDeltaY = 0
				if event.ControlDown():
					# toggle selection status of one object
					if hitObj.IsSelected():
						hitObj.Deselect()
					else:
						hitObj.Select()
				elif not hitObj.IsSelected():
					if not event.ShiftDown():
						for obj in self.Objects:
							obj.Deselect()
					hitObj.Select()
			else:
				# deselect all
				for obj in self.Objects:
					obj.Deselect()
		else:
			# mouse up
			for obj in self.Objects:
				if obj.IsSelected():
					p = obj.GetPosition()
					x = p[0]
					y = p[1]
					x += self.SelectDeltaX
					y += self.SelectDeltaY
					obj.SetPosition(x, y)
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
			for obj in self.Objects:
				if obj.IsSelected():
					anySelected = 1
					break
			if anySelected:
				self.drawArea.Refresh()

	# ==========================
	# == Menu Command Methods ==
	# ==========================

	def doNew(self, event):
		""" Respond to the "New" menu command.
		"""
		global _docList
		newFrame = DrawingFrame(None, -1, "Chromium Configuration Tool")
		newFrame.Show(TRUE)
		_docList.append(newFrame)


	def doOpen(self, event):
		""" Respond to the "Open" menu command.
		"""
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
			newFrame = DrawingFrame(None, -1, os.path.basename(fileName),
						fileName=fileName)
			newFrame.Show(true)
			_docList.append(newFrame)


	def doClose(self, event):
		""" Respond to the "Close" menu command.
		"""
		global _docList

		if self.dirty:
			if not self.askIfUserWantsToSave("closing"): return

		_docList.remove(self)
		self.Destroy()


	def doSave(self, event):
		""" Respond to the "Save" menu command.
		"""
		if self.fileName != None:
			self.saveContents()


	def doSaveAs(self, event):
		""" Respond to the "Save As" menu command.
		"""
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


	# Called by Delete menu item
	def doDelete(self, event):
		# Ugh, I can't find a Python counterpart to the C++ STL's remove_if()
		# function.
		# Have to make a temporary list of the objects to delete so we don't
		# screw-up the iteration as we remove things
		deleteList = []
		for obj in self.Objects:
			if obj.IsSelected():
				deleteList.append(obj)
		# now delete the objects in the deleteList
		for obj in deleteList:
			 self.Objects.remove(obj)
		self.drawArea.Refresh()

	# Called by Select All menu item
	def doSelectAll(self, event):
		for obj in self.Objects:
			obj.Select()
		self.drawArea.Refresh()

	# Called by the Connect menu item
	def doConnect(self, event):
		for obj in self.Objects:
			if obj.IsSelected():
				if not obj.IsServer() and obj.HasPacker():
					# Connect this app node to all selected servers
					for s in self.Objects:
						if s.IsServer() and s.IsSelected():
							self.AddConnection(obj, s)
							
		self.drawArea.Refresh()

	# Called by the Disconnect menu item
	def doDisconnect(self, event):
		# Make list of connections to remove
		removeList = []
		for conn in self.Connections:
			(fromNode, toNode) = conn
			if fromNode.IsSelected() and not conn in removeList:
				removeList.append(conn)
			elif toNode.IsSelected() and not conn in removeList:
				removeList.append(conn)
		# Now remove those connections
		for conn in removeList:
			self.Connections.remove(conn)
		self.drawArea.Refresh()

	# Called by the Edit/SetHost menu item
	def doSetHost(self, event):
		dialog = wxTextEntryDialog(self, "Enter the hostname for the selected nodes")
		if dialog.ShowModal() == wxID_OK:
			for obj in self.Objects:
				if obj.IsSelected():
					obj.SetHost(dialog.GetValue())
		dialog.Destroy()
		self.drawArea.Refresh()

	# Called by the Help/Introdunction menu item
	def doShowIntro(self, event):
		""" Respond to the "Introduction" menu command.
		"""
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
		""" Respond to the "About" menu command.
		"""
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
		dialog = wxMessageDialog(parent=self, message=msg, caption="Hey!", style=wxOK)
		dialog.ShowModal()


	# ======================
	# == File I/O Methods ==
	# ======================

	def loadContents(self):
		""" Load the contents of our document into memory.
		"""
		f = open(self.fileName, "rb")
		objData = cPickle.load(f)
		f.close()

#		for type, data in objData:
#			obj = DrawingObject(type)
#			obj.setData(data)
#			self.contents.append(obj)

		self.dirty = false

		self.drawArea.Refresh()


	def saveContents(self):
		""" Save the contents of our document to disk.
		"""
		objData = []
		for obj in self.contents:
			objData.append([obj.getType(), obj.getData()])

		f = open(self.fileName, "wb")
		cPickle.dump(objData, f)
		f.close()

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
	def __init__(self, name, packer=0):
		self.X = 0
		self.Y = 0
		self._Name = name
		self.Width = 0
		self.Height = 30
		self._IsPacker = packer
		self._IsSelected = 0

	# Return true if this SPU has a packer
	def IsPacker(self):
		return self._IsPacker

	# Return true if this SPU has to be the last in a chain (a terminal)
	def IsTerminal(self):
		return self._Name in TerminalSPUs

	def Select(self):
		self._IsSelected = 1

	def Deselect(self):
		self._IsSelected = 0

	def IsSelected(self):
		return self._IsSelected

	def PickTest(self, x, y):
		if x >= self.X and x < self.X + self.Width and y >= self.Y and y < self.Y + self.Height:
			return 1
		else:
			return 0

	def SetPosition(self, x, y):
		self.X = x
		self.Y = y

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
		if self._IsPacker:
			dc.SetBrush(wxBLACK_BRUSH)
			dc.DrawRectangle(self.X + self.Width, self.Y + self.Height/2 - 4, 4, 8)

	def Name(self):
		return self._Name

	def GetPosition(self):
		return (self.X, self.Y)

	def GetWidth(self):
		return self.Width

	def GetHeight(self):
		return self.Height

class NodeObject:
	""" The graphical representation of a Cr node.
	"""

	def __init__(self, host="localhost", x=100, y=100, isServer = 0):
		self.X = x
		self.Y = y
		self.Width = 200
		self.Height = 60
		self.SpuChain = []
		self._IsServer = isServer
		self._IsSelected = 0
		self._InputPlugPos = (0,0)
		self._Host = host

	# Return true if this is a server, false otherwise
	def IsServer(self):
		return self._IsServer

	# Return true if the last SPU has a packer:
	def HasPacker(self):
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
			frame = DrawingFrame(None, -1, "Chromium Configuration Tool")
			frame.Centre()
			frame.Show(TRUE)
			_docList.append(frame)
		else:
			# Load the file(s) specified on the command line.
			for arg in sys.argv[1:]:
				fileName = os.path.join(os.getcwd(), arg)
				if os.path.isfile(fileName):
					frame = DrawingFrame(None, -1,
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


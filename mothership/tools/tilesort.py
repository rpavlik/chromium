""" Tilesort.py

    Tool for making Chromium tilesort configurations.

    Useful info to know about wxWindows/wxPython:

	There's kind of a dual hierarchy to the UI elements: the window hierarcy
	and the layout hierarchy.  When you create a control (i.e. widget) you have
	to specify a parent window as one of the parameters.  That adds the child
	to the parent.
	
	You also need to create "sizers" which regulate the layout of controls.
	You can add controls (and windows) to sizers with the Add() method.
	At some point you specify the sizer for a window with wxWindow::SetSizer().

	wxControl is derived from wxWindow.
	wxWindow is derived from wxEvtHandler.
	wxEvtHandler is derived from wxObject.

	wxSizer is derived from wxObject.

	Another clunky bit is the fact that you have to specify an ID number for
	all wxControls.  The ID is used to hook event handlers to controls.
	I don't know why the handle returned from creating a control couldn't be
	used instead of the IDs.
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

menu_ABOUT         = 10205 # Help menu items.

# Widget IDs
id_MuralWidth  = 3000
id_MuralHeight = 3001
id_TileChoice  = 3002
id_TileWidth   = 3003
id_TileHeight  = 3004
id_hLayout     = 3005
id_vLayout     = 3006

# Size of the drawing page, in pixels.
PAGE_WIDTH  = 1000
PAGE_HEIGHT = 1000

CommonTileSizes = [ [128, 128],
					[256, 256],
					[512, 512],
					[1024, 1024],
					[1280, 1024],
					[1600, 1200] ]


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
		self.editMenu.Append(menu_SELECT_ALL,    "Select All\tCTRL-A")
		self.editMenu.AppendSeparator()
		menuBar.Append(self.editMenu, "Edit")

		# Help menu
		self.helpMenu = wxMenu()
		self.helpMenu.Append(menu_ABOUT, "About Tilesort Config...")
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

		toolSizer = wxBoxSizer(wxVERTICAL)

		# Mural width/height (in tiles)
		box = wxStaticBox(parent=self.topPanel, id=-1, label="Mural Size",
						  style=wxDOUBLE_BORDER)
		muralSizer = wxStaticBoxSizer(box, wxVERTICAL)
		flexSizer = wxFlexGridSizer(rows=2, cols=2, hgap=4, vgap=4)
		columnsLabel = wxStaticText(parent=self.topPanel, id=-1, label="Columns:")
		self.widthControl = wxSpinCtrl(parent=self.topPanel, id=id_MuralWidth,
									   value="4", min=1, max=16, size=wxSize(50,25))
		rowsLabel = wxStaticText(parent=self.topPanel, id=-1, label="Rows:")
		self.heightControl = wxSpinCtrl(parent=self.topPanel, id=id_MuralHeight,
										value="3", min=1, max=16, size=wxSize(50,25))
		EVT_SPINCTRL(self.widthControl, id_MuralWidth, self.onSizeChange)
		EVT_SPINCTRL(self.heightControl, id_MuralHeight, self.onSizeChange)
		flexSizer.Add(columnsLabel)
		flexSizer.Add(self.widthControl)
		flexSizer.Add(rowsLabel)
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
		self.tileWidthControl = wxSpinCtrl(parent=self.topPanel, id=id_TileWidth,
										   value="512", min=128, max=2048,
										   size=wxSize(80,25))
		self.tileHeightLabel = wxStaticText(parent=self.topPanel, id=-1,
											label="Height:")
		self.tileHeightControl = wxSpinCtrl(parent=self.topPanel, id=id_TileHeight,
											value="512", min=128, max=2048,
											size=wxSize(80,25))
		EVT_SPINCTRL(self.tileWidthControl, id_TileWidth, self.onSizeChange)
		EVT_SPINCTRL(self.tileHeightControl, id_TileHeight, self.onSizeChange)
		EVT_CHOICE(self.tileChoice, id_TileChoice, self.onTileChoice)
		flexSizer.Add(self.tileWidthLabel)
		flexSizer.Add(self.tileWidthControl)
		flexSizer.Add(self.tileHeightLabel)
		flexSizer.Add(self.tileHeightControl)
		tileSizer.Add(self.tileChoice, flag=wxALIGN_CENTER)
		tileSizer.Add(flexSizer)
		toolSizer.Add(tileSizer, flag=wxEXPAND)

		# Total mural size (in pixels)
		box = wxStaticBox(parent=self.topPanel, id=-1, label="Total Size", style=wxDOUBLE_BORDER)
		totalSizer = wxStaticBoxSizer(box, wxVERTICAL)
		self.totalSizeLabel = wxStaticText(parent=self.topPanel, id=-1, label="??")
		totalSizer.Add(self.totalSizeLabel, flag=wxEXPAND)
		toolSizer.Add(totalSizer, flag=wxEXPAND)

		hChoices = [ 'Left to right', 'Right to left' ]
		self.hLayoutRadio = wxRadioBox(parent=self.topPanel, id=id_hLayout,
									   label="Horizontal Layout", choices=hChoices,
									   majorDimension=1, style=wxRA_SPECIFY_COLS )
		toolSizer.Add(self.hLayoutRadio, flag=wxEXPAND)
		vChoices = [ 'Top to bottom', 'Bottom to top' ]
		self.vLayoutRadio = wxRadioBox(parent=self.topPanel, id=id_vLayout,
									   label="Vertical Layout", choices=vChoices,
									   majorDimension=1, style=wxRA_SPECIFY_COLS )
		toolSizer.Add(self.vLayoutRadio, flag=wxEXPAND)
		EVT_RADIOBOX(self.hLayoutRadio, id_hLayout, self.onSizeChange)
		EVT_RADIOBOX(self.vLayoutRadio, id_vLayout, self.onSizeChange)

		# Setup the main drawing area.
#		self.drawArea = wxScrolledWindow(self.topPanel, -1,
#										 style=wxSUNKEN_BORDER)
#		self.drawArea.EnableScrolling(true, true)
#		self.drawArea.SetScrollbars(20, 20, PAGE_WIDTH / 20, PAGE_HEIGHT / 20)
		self.drawArea = wxPanel(self.topPanel, id=-1, style=wxSUNKEN_BORDER)
		self.drawArea.SetBackgroundColour(wxWHITE)
		EVT_PAINT(self.drawArea, self.onPaintEvent)

		# Position everything in the window.
		topSizer = wxBoxSizer(wxHORIZONTAL)
		topSizer.Add(toolSizer, 0, wxTOP | wxLEFT | wxRIGHT | wxALIGN_TOP, 5)
		topSizer.Add(self.drawArea, 1, wxEXPAND)

		self.topPanel.SetAutoLayout(true)
		self.topPanel.SetSizer(topSizer)

		self.SetSizeHints(minW=250, minH=200)
		self.SetSize(wxSize(600, 400))

		self.dirty     = false
		self.fileName  = fileName

		self.recomputeTotalSize()
		
		if self.fileName != None:
			self.loadContents()


	# This is called whenever the mural width/height or tile width/height changes.
	# We recompute the total mural size in pixels and update the widgets.
	def recomputeTotalSize(self):
		tileW = self.tileWidthControl.GetValue()
		tileH = self.tileHeightControl.GetValue()
		totalW = self.widthControl.GetValue() * tileW
		totalH = self.heightControl.GetValue() * tileH
		self.totalSizeLabel.SetLabel(str("%d x %d" % (totalW, totalH)))
		custom = 1
		for i in range(0, len(CommonTileSizes)):
			if tileW == CommonTileSizes[i][0] and tileH == CommonTileSizes[i][1]:
				self.tileChoice.SetSelection(i)
				return
		# must be custom size
		self.tileChoice.SetSelection(len(CommonTileSizes))  # "Custom"

		
	# ============================
	# == Event Handling Methods ==
	# ============================

	def onSizeChange(self, event):
		"""Respond to spin control changes"""
		self.recomputeTotalSize()
		self.drawArea.Refresh()

	def onTileChoice(self, event):
		""" Respond to the "Tile Size" choice widget.
		"""
		i = self.tileChoice.GetSelection()
		if i < len(CommonTileSizes):
			w = CommonTileSizes[i][0]
			h = CommonTileSizes[i][1]
			self.tileWidthControl.SetValue(w)
			self.tileHeightControl.SetValue(h)
		self.recomputeTotalSize()
		self.drawArea.Refresh()


	def onPaintEvent(self, event):
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
				dc.DrawText("cr%d" % k, x+3, y+3)
		dc.EndDrawing()


	# ==========================
	# == Menu Command Methods ==
	# ==========================

	def doNew(self, event):
		""" Respond to the "New" menu command.
		"""
		global _docList
		newFrame = DrawingFrame(None, -1, "Chromium Tilesort Configuration")
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

class TilesortApp(wxApp):
	""" The main Tilesort application object.
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
			frame = DrawingFrame(None, -1, "Chromium Tilesort Configuration")
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
	""" Start up the Tilesort application.
	"""
	global _app

	# Redirect python exceptions to a log file.

	sys.stderr = ExceptionHandler()

	# Create and start the Tilesort application.

	_app = TilesortApp(0)
	_app.MainLoop()


if __name__ == "__main__":
	main()


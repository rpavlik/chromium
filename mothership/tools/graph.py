#!/usr/bin/env python
# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

""" graph.py

This module defines the GraphFrame class which is the top-level window
for creating/editing Chromium graphs.
"""


import string, os, signal, os.path, types, re, random, copy
from wxPython.wx import *

import crtypes, crutils
import configio
import spudialog, intdialog, hostdialog, tiledialog, textdialog
import tilesort_template
import sortlast_template
import reassembly_template

# Global vars
App = None


#----------------------------------------------------------------------
# Templates

# Map template name strings to classes
__Templates = {
	"Tilesort"         : tilesort_template.TilesortTemplate,
	"Sort-last"        : sortlast_template.SortlastTemplate,
	"Image Reassembly" : reassembly_template.ReassemblyTemplate,
}

def GetTemplateList():
	"""Return list of names of known templates."""
	return __Templates.keys()

def InstantiateTemplate(templateName):
	"""Return a new instance of the named template class."""
	tClass = __Templates[templateName]
	if tClass:
		return tClass()
	else:
		return None


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
menu_COMBINE_NODES      = 211
menu_SERVER_OPTIONS     = 212
menu_APP_OPTIONS        = 213
menu_SERVER_TILES       = 214

menu_SELECT_ALL_SPUS      = 300
menu_SELECT_ALL_SPUS_TYPE = 301
menu_DESELECT_ALL_SPUS    = 302
menu_DELETE_SPU           = 303
menu_SPU_OPTIONS          = 304
menu_SPU_TYPES            = 320

menu_LAYOUT_NODES         = 400

menu_MOTHERSHIP_OPTIONS   = 500
menu_MOTHERSHIP_RUN       = 501
menu_MOTHERSHIP_STOP      = 502

menu_ABOUT              = 600
menu_DOCS               = 601


# Widget IDs
id_NewServerNode  = 3000
id_NewAppNode     = 3001
id_NewSpu         = 3002
id_NewTemplate    = 3003
id_TemplateOptions = 3004

# Initial size of the drawing page, in pixels.
PAGE_WIDTH  = 1000
PAGE_HEIGHT = 1000

# Window title (gets appended with project name)
TitleString = "Chromium Configuration Tool"

# Config file dialog filename filter
WildcardPattern = "Chromium Configs (*.conf)|*.conf|All (*)|*"

# Brushes and pens for drawing nodes and SPUs
_AppNodeBrush = wxBrush(wxColor(55, 160, 55))
_ServerNodeBrush = wxBrush(wxColor(210, 105, 135))
_BackgroundColor = wxColor(90, 150, 190)
_ThinBlackPen = wxPen(wxColor(0,0,0), width=1, style=0)
_WideBlackPen = wxPen(wxColor(0,0,0), width=3, style=0)
_WirePen = wxPen(wxColor(0, 0, 250), width=2, style=0)
_SPUBrush = wxLIGHT_GREY_BRUSH

NodeClipboard = []


#----------------------------------------------------------------------------
# Main window frame class for editing Chromium graphs

class GraphFrame(wxFrame):
	"""This is a frame for editing a Chromium graph / mothership config."""

	def __init__(self, parent, id, title):
		"""parent, id, title are the usual wxFrame parameters."""
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
		#self.editMenu.Append(wxID_UNDO,   "Undo\tCTRL-Z")
		#self.editMenu.Append(wxID_REDO,   "Redo\tSHIFT-CTRL-Z")
		#self.editMenu.AppendSeparator()
		self.editMenu.Append(wxID_CUT,    "Cut\tCTRL-X")
		self.editMenu.Append(wxID_COPY,   "Copy\tCTRL-C")
		self.editMenu.Append(wxID_PASTE,  "Paste\tCTRL-V")
		EVT_MENU(self, wxID_UNDO, self.doUndo)
		EVT_MENU(self, wxID_REDO, self.doRedo)
		EVT_MENU(self, wxID_CUT, self.doCut)
		EVT_MENU(self, wxID_COPY, self.doCopy)
		EVT_MENU(self, wxID_PASTE, self.doPaste)
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
		self.nodeMenu.Append(menu_SET_HOST,           "Host name...")
		self.nodeMenu.Append(menu_SET_COUNT,          "Set Count...")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_SPLIT_NODES,        "Split")
		self.nodeMenu.Append(menu_COMBINE_NODES,      "Combine")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_SERVER_OPTIONS,     "Server Node Options...")
		self.nodeMenu.Append(menu_SERVER_TILES,       "Server Node Tiles...")
		self.nodeMenu.AppendSeparator()
		self.nodeMenu.Append(menu_APP_OPTIONS,        "App Node Options...")
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
		EVT_MENU(self, menu_COMBINE_NODES, self.doCombineNodes)
		EVT_MENU(self, menu_SERVER_OPTIONS, self.doServerOptions)
		EVT_MENU(self, menu_SERVER_TILES, self.doServerTiles)
		EVT_MENU(self, menu_APP_OPTIONS, self.doAppOptions)
		EVT_MENU(self, menu_LAYOUT_NODES, self.doLayoutNodes)
		menuBar.Append(self.nodeMenu, "Node")

		spuClasses = crutils.FindSPUNames()

		# SPU types menu
		self.spuTypesMenu = wxMenu()
		i = 0
		for spu in spuClasses:
			self.spuTypesMenu.Append(menu_SPU_TYPES + i, spu)
			EVT_MENU(self, menu_SPU_TYPES + i, self.doSelectAllSPUsByType)
			i += 1

		# SPU menu
		self.spuMenu = wxMenu()
		self.spuMenu.AppendSeparator()
		self.spuMenu.Append(menu_SELECT_ALL_SPUS,      "Select All ")
		self.spuMenu.AppendMenu(menu_SELECT_ALL_SPUS_TYPE,
							"Select All by Type",
							self.spuTypesMenu)
		self.spuMenu.Append(menu_DESELECT_ALL_SPUS,    "Deselect All")
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
		self.systemMenu.Append(menu_MOTHERSHIP_OPTIONS, "Options...")
		self.systemMenu.Append(menu_MOTHERSHIP_RUN, "Run...")
		self.systemMenu.Append(menu_MOTHERSHIP_STOP, "Stop...")
		self.systemMenu.Enable(menu_MOTHERSHIP_STOP, 0) #disable
		EVT_MENU(self, menu_MOTHERSHIP_OPTIONS, self.doMothershipOptions)
		EVT_MENU(self, menu_MOTHERSHIP_RUN, self.doMothershipRun)
		EVT_MENU(self, menu_MOTHERSHIP_STOP, self.doMothershipStop)
		menuBar.Append(self.systemMenu, "Mothership")

		# Help menu
		self.helpMenu = wxMenu()
		self.helpMenu.Append(menu_ABOUT, "About Config tool...")
		self.helpMenu.Append(menu_DOCS,  "Documentation...")
		EVT_MENU(self, menu_ABOUT, self.doShowAbout)
		EVT_MENU(self, menu_DOCS, self.doShowDocs)
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

		templateNames = [ "New Template" ] + GetTemplateList()
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
		spuStrings = ["New SPU"] + spuClasses
		self.newSpuChoice = wxChoice(parent=self.topPanel, id=id_NewSpu,
									 choices=spuStrings)
		EVT_CHOICE(self.newSpuChoice, id_NewSpu, self.onNewSpu)
		toolSizer.Add(self.newSpuChoice, flag=wxEXPAND+wxALL, border=2)

		# Setup the main drawing area.
		self.drawArea = wxScrolledWindow(self.topPanel, -1,
										 style=wxSUNKEN_BORDER)
		self.drawArea.EnableScrolling(true, true)
		self.drawArea.SetScrollbars(20, 20, PAGE_WIDTH / 20, PAGE_HEIGHT / 20)
		self.drawArea.SetBackgroundColour(_BackgroundColor)
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


		self.dirty = false
		self.fileName = None
		self.mothership = crtypes.Mothership() # a new mothership object
		self.undoStack = []
		self.redoStack = []
		self.LeftDown = 0
		self.DragStartX = 0
		self.DragStartY = 0
		self.SelectDeltaX = 0
		self.SelectDeltaY = 0
		self.__FontHeight = 0
		self._mothershipProcess = None

		self.UpdateMenus()

		# Create the help/documentation dialog
		self.helpDialog = textdialog.TextDialog(parent=self, id = -1,
									   title="Config Tool Documentation")
		self.helpDialog.LoadPage("../../doc/configtool.html")


	# ----------------------------------------------------------------------
	# Utility functions

	def SaveState(self):
		"""Save the current state to the undo stack.  This should be
		called immediately before any modifications to the project."""
		pass # XXX temporary - deepcopy() is broken!
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

	def __drawSPU(self, dc, spu):
		"""Draw icon representation of the given SPU."""
		dc.SetBrush(_SPUBrush)
		if spu.IsSelected():
			dc.SetPen(_WideBlackPen)
		else:
			dc.SetPen(_ThinBlackPen)
		# if width is zero, compute width/height now
		(w, h) = spu.GetSize()
		if w == 0:
			spu.Layout(dc.GetTextExtent)
			(w, h) = spu.GetSize()
		(x, y) = spu.GetPosition()
		# draw the SPU as a rectangle with text label
		dc.DrawRectangle(x, y, w, h)
		dc.DrawText(spu.Name(), x + 4, y + 4)
		if spu.MaxServers() > 0:
			# draw the output plug (a little black rect)
			dc.SetBrush(wxBLACK_BRUSH)
			dc.DrawRectangle(x + w, y + h / 2 - 5, 4, 10)
		elif spu.IsTerminal():
			# draw a thick right edge on the box
			dc.SetPen(_WideBlackPen)
			dc.DrawLine(x + w, y + 1, x + w, y + h - 2)

	def __drawNode(self, dc, node, dx=0, dy=0):
		"""Draw icon representation of the given node."""
		if self.__FontHeight == 0:
			(spam, self.__FontHeight) = dc.GetTextExtent("spam")
		# set brush and pen
		if node.IsServer():
			dc.SetBrush(_ServerNodeBrush)
		else:
			dc.SetBrush(_AppNodeBrush)
		if node.IsSelected():
			dc.SetPen(_WideBlackPen)
		else:
			dc.SetPen(_ThinBlackPen)
		# get node's position
		(x, y) = node.GetPosition()
		# add temporary offset used while dragging nodes
		x += dx
		y += dy
		# get node's size
		(w, h) = node.GetSize();
		if w == 0 or h == 0:
			node.Layout(dc.GetTextExtent)
			(w, h) = node.GetSize();

		# draw the node's box
		if node.GetCount() > 1:
			# draw the "Nth box"
			dc.DrawRectangle(x + 8, y + self.__FontHeight + 4, w, h)
			dc.DrawText(" ... Count = %d" % node.GetCount(),
						x + 12, y + h + 1 )
		dc.DrawRectangle(x, y, w, h)
		if node.IsServer():
			dc.DrawText(node.GetLabel(), x + 4, y + 4)
			# draw the unpacker plug
			px = x - 4
			py = y + h / 2
			dc.SetBrush(wxBLACK_BRUSH)
			dc.DrawRectangle(px, py - 5, 4, 10)
		else:
			dc.DrawText(node.GetLabel(), x + 4, y + 4)

		# draw the SPUs
		x = x + 5
		y = y + 20
		for spu in node.SPUChain():
			spu.SetPosition(x, y)
			self.__drawSPU(dc, spu)
			(w, h) = spu.GetSize()
			x += w + 2


	def onPaintEvent(self, event):
		"""Drawing area repaint callback"""
		dc = wxPaintDC(self.drawArea)
		self.drawArea.PrepareDC(dc)  # only for scrolled windows
		dc.BeginDrawing()

		# draw the nodes
		for node in self.mothership.Nodes():
			if node.IsSelected():
				self.__drawNode(dc, node, self.SelectDeltaX, self.SelectDeltaY)
			else:
				self.__drawNode(dc, node)

		# draw the wires between the nodes
		dc.SetPen(_WirePen)
		for node in self.mothership.Nodes():
			servers = node.GetServers()
			for serv in servers:
				(px, py) = node.GetOutputPlugPos()
				(qx, qy) = serv.GetInputPlugPos()
				if serv.IsSelected():
					qx += self.SelectDeltaX
					qy += self.SelectDeltaY
				dc.DrawLine( px, py, qx, qy)
				# See if we need to draw triple lines for N-hosts
				dy1 = (node.GetCount() > 1) * 4
				dy2 = (serv.GetCount() > 1) * 4
				if dy1 != 0 or dy2 != 0:
					dc.DrawLine(px, py + dy1, qx, qy + dy2)
					dc.DrawLine(px, py - dy1, qx, qy - dy2)
		# all done drawing
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
							   defaultValues=[2], minValue=1, maxValue=1000)
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
		self.dirty = true

	# called when New Server Node button is pressed
	def onNewServerNode(self, event):
		self.SaveState()
		self.mothership.DeselectAllNodes()
		xstart = random.randrange(250, 300, 5)
		ystart = random.randrange(50, 100, 5)
		n = self.newServerChoice.GetSelection()
		if n < 5:
			# generate the hosts
			hosts = crutils.GetSiteDefault("cluster_hosts")
			if not hosts:
				hosts = ["localhost"]
			pattern = crutils.GetSiteDefault("cluster_pattern")
			# generate the nodes and initialize them
			for i in range(0, n):
				node = crutils.NewNetworkNode()
				node.SetPosition(xstart, ystart + i * 65)
				if i < len(hosts):
					node.SetHosts( [ hosts[i] ] )
				else:
					node.SetHosts( [ hosts[-1] ] )
				if pattern:
					node.SetHostNamePattern(pattern)
				node.Select()
				self.mothership.AddNode(node)
		else:
			dialog = intdialog.IntDialog(parent=NULL, id=-1,
							   title="Create Server Nodes",
							   labels=["Number of Server nodes:"],
							   defaultValues=[4], minValue=1, maxValue=1000)
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
		self.dirty = true

	def onNewSpu(self, event):
		"""New SPU button callback"""
		# add a new SPU to all selected nodes
		i = self.newSpuChoice.GetSelection()
		if i <= 0:
			return # didn't really select an SPU class
		self.SaveState()
		i -= 1
		spuClasses = crutils.FindSPUNames()
		for node in self.mothership.SelectedNodes():
			# first check that this kind of SPU isn't already in the chain
			dup = 0
			for spu in node.SPUChain():
				if spu.Name() == spuClasses[i]:
					self.Notify("Only one %s SPU is allowed per node." %
								spuClasses[i])
					dup = 1
					break
			if dup:
				continue
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
							(spuClasses[i], pred.Name()))
				break
			# check if it's legal to put this SPU before another
			if pos >= 0 and crutils.SPUIsTerminal(spuClasses[i]):
				self.Notify("You can't insert a %s SPU before a %s SPU." %
							(spuClasses[i], node.GetSPU(pos).Name()))
				break
			# OK, we're all set, add the SPU
			s = crutils.NewSPU( spuClasses[i] )
			node.AddSPU(s, pos)
		self.drawArea.Refresh()
		self.newSpuChoice.SetSelection(0)
		self.dirty = true

	def onNewTemplate(self, event):
		"""New Template button callback"""
		self.SaveState()
		t = self.newTemplateChoice.GetSelection()
		if t > 0:
			templateName = GetTemplateList()[t - 1]
			assert templateName != ""
			t = InstantiateTemplate(templateName)
			assert t
			if t.Create(self, self.mothership):
				self.mothership.SetTemplateType(t)
		self.newTemplateChoice.SetSelection(0)
		self.drawArea.Refresh()
		self.UpdateMenus()
		self.dirty = true

	def onTemplateEdit(self, event):
		t = self.mothership.GetTemplateType()
		if t:
			t.Edit(self, self.mothership)
		else:
			self.Notify("This configuration doesn't match any template.")
		self.drawArea.Refresh()
		self.UpdateMenus()
		self.dirty = true

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
			if (self.SelectDeltaX != 0 and self.SelectDeltaY != 0
				and len(self.mothership.SelectedNodes()) > 0):
				self.SaveState()
				self.dirty = true
				# translate the selected nodes
				for node in self.mothership.SelectedNodes():
					p = node.GetPosition()
					x = p[0] + self.SelectDeltaX
					y = p[1] + self.SelectDeltaY
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
		newFrame = GraphFrame(None, -1, TitleString)
		newFrame.Show(TRUE)
		App.DocList.append(newFrame)


	def doOpen(self, event):
		"""File / Open callback"""

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
			if self.loadConfig(fileName):
				title = TitleString + ": " + os.path.basename(fileName)
				self.SetTitle(title)
			else:
				self.fileName = None
		else:
			# Open a new frame for this document.
			title = TitleString + ": " + os.path.basename(fileName)
			newFrame = GraphFrame(None, -1, title)
			if newFrame.loadConfig(fileName):
				newFrame.Show(true)
				App.DocList.append(newFrame)
			else:
				newFrame.Destroy()


	def doClose(self, event):
		"""File / Close callback"""
		if self.dirty:
			if not self.askIfUserWantsToSave("closing"):
				return

		App.DocList.remove(self)
		if len(App.DocList) == 0:
			App.ExitMainLoop()

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
		del self.mothership
		self.mothership = crtypes.Mothership() # a new mothership object
		self.loadConfig(self.fileName)


	def doExit(self, event):
		""" Respond to the "Quit" menu command.
		"""
		docs = App.DocList[:]  # make a copy since we call remove() in the loop
		for doc in docs:
			if doc.dirty:
				doc.Raise()
				if not doc.askIfUserWantsToSave("quitting"):
					return
			App.DocList.remove(doc)
			doc.Destroy()

		App.ExitMainLoop()

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

	def doCut(self, event):
		"""Edit / Cut callback."""
		# Remove selected nodes from graph, put on clipboard
		global NodeClipboard
		cutNodes = self.mothership.SelectedNodes()
		crutils.RemoveNodesFromList(self.mothership.Nodes(), cutNodes)
		NodeClipboard = cutNodes
		for node in cutNodes:
			node.Deselect()
		self.drawArea.Refresh()
		self.UpdateMenus()
		self.dirty = true

	def doCopy(self, event):
		"""Edit / Copy callback."""
		# Copy selected nodes to clipboard
		global NodeClipboard
		copyNodes = self.mothership.SelectedNodes()
		NodeClipboard = crutils.CloneNodeList(copyNodes)
		for node in NodeClipboard:
			node.Deselect()
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doPaste(self, event):
		"""Edit / Paste callback."""
		# Copy selected nodes from clipboard to mothership
		global NodeClipboard
		self.mothership.DeselectAllNodes()
		newNodes = crutils.CloneNodeList(NodeClipboard)
		for node in newNodes:
			node.Select()
			# translate a little
			p = node.GetPosition()
			px = p[0] + 10
			py = p[1] + 10
			node.SetPosition(px, py)
			self.mothership.AddNode(node)
		self.drawArea.Refresh()
		self.UpdateMenus()
		self.dirty = true

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
		## loop over nodes again to remove server connections
		#for node in self.mothership.Nodes():
		#	for server in node.GetServers():
		#		if server.IsSelected():
		#			node.LastSPU().RemoveServer(server)
		## now delete the objects in the deleteList
		#for node in deleteList:
		#	 self.mothership.Nodes().remove(node)

		crutils.RemoveNodesFromList(self.mothership.Nodes(), deleteList)
		
		self.drawArea.Refresh()
		self.UpdateMenus()
		self.dirty = true

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
			self.Notify("Can't connect - no available packing SPUs found.")
			#print "no packers!"
			return

		# see if we need to reassign any packers as servers.
		# solving this situation is done via an ad hoc heuristic.
		if len(serverNodes) == 0:
			# reassign a network packer as a server
			if len(appPackerNodes) > 0 and len(netPackerNodes) > 0:
				# Change all netPacker nodes into servers
				serverNodes = netPackerNodes[:]
				netPackerNodes = []
			elif len(netPackerNodes) > 1:
				# Change one packer into a server
				# sort the packer nodes by position
				self.mothership.SortNodesByPosition(netPackerNodes)
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
			elif len(netPackerNodes) >= 1 and len(appPackerNodes) >= 1:
				# Change a netPacker node into server node
				server = netPackerNodes[0]
				netPackerNodes.remove(server)
				serverNodes.append(server)
			else:
				self.Notify("Can't connect - no available server nodes found.")
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
		self.dirty = true

	def doDisconnect(self, event):
		"""Node / Disconnect callback"""
		self.SaveState()
		numDisconnected = 0
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
					numDisconnected += 1
		if numDisconnected == 0:
			self.Notify("Didn't find any connections.")
		self.drawArea.Refresh()
		self.UpdateMenus()
		self.dirty = true

	def doLayoutNodes(self, event):
		"""Node / Layout Nodes callback"""
		self.SaveState()
		self.mothership.LayoutNodes()
		self.drawArea.Refresh()
		self.dirty = true

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
		self.dirty = true

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
		self.dirty = true

	def doSplitNodes(self, event):
		"""Node / Split callback"""
		for node in self.mothership.SelectedNodes():
			if node.GetCount() > 1:
				crutils.SplitNode(node, self.mothership)
		self.drawArea.Refresh()
		self.UpdateMenus()
		self.dirty = true

	def doCombineNodes(self, event):
		"""Node / Combine callback"""
		nodes = self.mothership.SelectedNodes()
		if not crutils.MergeNodes(nodes, self.mothership):
			self.Notify("The selected nodes are too dissimilar to be combined.")
		self.drawArea.Refresh()
		self.UpdateMenus()
		self.dirty = true

	def doServerOptions(self, event):
		"""Node / Server Options callback"""
		serverNode = 0
		for node in self.mothership.SelectedNodes():
			if node.IsServer():
				serverNode = node
				break
		assert serverNode
		dialog = spudialog.SPUDialog(parent=self, id=-1,
								title="Server Node Options",
								optionList=serverNode.GetOptions())
		dialog.Centre()
		if dialog.ShowModal() == wxID_OK:
			for opt in serverNode.GetOptions():
				value = dialog.GetValue(opt.Name)
				for node in self.mothership.SelectedNodes():
					if node.IsServer():
						node.SetOption(opt.Name, value)
			#endfor
			self.dirty = true

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
									   hosts = server.GetHosts(),
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
			self.dirty = true

	def doAppOptions(self, event):
		"""Node / App Options callback"""
		appNode = 0
		for node in self.mothership.SelectedNodes():
			if node.IsAppNode():
				appNode = node
				break
		assert appNode
		dialog = spudialog.SPUDialog(parent=self, id=-1,
								title="Application Node Options",
								optionList=appNode.GetOptions())
		dialog.Centre()
		if dialog.ShowModal() == wxID_OK:
			for opt in appNode.GetOptions():
				value = dialog.GetValue(opt.Name)
				for node in self.mothership.SelectedNodes():
					if node.IsAppNode():
						node.SetOption(opt.Name, value)
			#endfor
			self.dirty = true


	# ----------------------------------------------------------------------
	# SPU menu callbacks

	def doSelectAllSPUs(self, event):
		"""SPU / Select All callback"""
		for node in self.mothership.SelectedNodes():
			for spu in node.SPUChain():
				spu.Select()
		self.drawArea.Refresh()
		self.UpdateMenus()

	def doSelectAllSPUsByType(self, event):
		"""SPU / Select All by Type callback"""
		i = event.GetId() - menu_SPU_TYPES
		spuClasses = crutils.FindSPUNames()
		spuName = spuClasses[i]
		for node in self.mothership.Nodes():
			for spu in node.SPUChain():
				if spu.Name() == spuName:
					spu.Select()
				else:
					spu.Deselect()
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
		self.dirty = true

	def doSpuOptions(self, event):
		"""SPU / SPU Options callback"""
		# find first selected SPU
		spuList = self.mothership.GetSelectedSPUs()
		if len(spuList) > 0:
			# check for a homogeneous list of SPUs
			homogeneous = 1
			for spu in spuList:
				if spu.Name() != spuList[0].Name():
					homogeneous = 0
			if not homogeneous:
				self.Notify("Can't edit SPU options for a " +
							"heterogeneous list of SPUs.")
				return

			name = spuList[0].Name()
			spuClasses = crutils.FindSPUNames()
			if name in spuClasses:
				# NOTE: we use the 0th SPU's option list to initialize
				# the options in the dialog.
				optionlist = spuList[0].GetOptions()
				# create the dialog
				dialog = spudialog.SPUDialog(parent=self, id=-1,
											 title=name + " SPU Options",
											 optionList = optionlist)
				dialog.Centre()
				# wait for OK or cancel
				if dialog.ShowModal() == wxID_OK:
					# save the new values/options in all selected SPUs
					for spu in spuList:
						for opt in optionlist:
							value = dialog.GetValue(opt.Name)
							spu.SetOption(opt.Name, value)
				else:
					# user cancelled, do nothing, new values are ignored
					pass
			else:
				print "Invalid SPU name: %s !!!" % name
		self.dirty = true
		return
		
	# ----------------------------------------------------------------------
	# Mothership menu callbacks
	
	def doMothershipOptions(self, event):
		"""Mothership / Options callback"""
		dialog = spudialog.SPUDialog(parent=self, id=-1,
								title="Mothership Options",
								optionList=self.mothership.GetOptions())
		dialog.Centre()
		if dialog.ShowModal() == wxID_OK:
			for opt in self.mothership.GetOptions():
				value = dialog.GetValue(opt.Name)
				self.mothership.SetOption(opt.Name, value)
			self.dirty = true

	def doMothershipRun(self, event):
		"""Run the mothership"""
		if self._mothershipProcess != None:
			# kill previous instance
			os.kill(self._mothershipProcess, signal.SIGKILL)
		print "spawning python"
		# write config to a temp file
		f = open("temp.conf", "w")
		if not f:
			print "Error writing temporary file: temp.conf"
			return
		configio.WriteConfig(self.mothership, f)
		f.close()
		# enable the "stop" menu item
		self.systemMenu.Enable(menu_MOTHERSHIP_STOP, 1)
		if 1:
			# this works on linux but the python docs say that spawnvp
			# won't work on windows. <sigh>
			self._mothershipProcess = os.spawnvp( os.P_NOWAIT, "python", ["python", "temp.conf"] )
		else:
			# This doesn't seem to work on linux
			self._mothershipProcess = os.fork()
			if self._mothershipProcess == 0:
				# I'm the child
				os.execvp( "python", ["python", "foo.conf"] )
		print "Mothership process = %d" % self._mothershipProcess

	def doMothershipStop(self, event):
		"""Stop the mothership"""
		if self._mothershipProcess != None:
			print "Killing process %d" % self._mothershipProcess
			os.kill(self._mothershipProcess, signal.SIGKILL)
			self._mothershipProcess = None
		# disable menu item
		self.systemMenu.Enable(menu_MOTHERSHIP_STOP, 0)


	# ----------------------------------------------------------------------
	# Help menu callbacks

	def doShowDocs(self, event):
		"""Help / Documentation callback"""
		self.helpDialog.Show()

	def doShowAbout(self, event):
		"""Help / About callback"""
		dialog = wxDialog(self, -1, "About")
		dialog.SetBackgroundColour(wxWHITE)

		panel = wxPanel(dialog, -1)
		panelSizer = wxBoxSizer(wxVERTICAL)

		text = wxStaticText(parent=panel, id=-1, label=
					"Chromium 1.7 graphical configuration tool\n")

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
			self.editMenu.Enable(wxID_CUT, 1)
			self.editMenu.Enable(wxID_COPY, 1)
			self.nodeMenu.Enable(menu_DELETE_NODE, 1)
			self.nodeMenu.Enable(menu_DISCONNECT, 1)
			self.nodeMenu.Enable(menu_SET_HOST, 1)
			self.nodeMenu.Enable(menu_SET_COUNT, 1)
			self.nodeMenu.Enable(menu_SPLIT_NODES, 1)
			if self.mothership.NumSelectedNodes() > 1:
				self.nodeMenu.Enable(menu_COMBINE_NODES, 1)
				self.nodeMenu.Enable(menu_CONNECT, 1)
			else:
				self.nodeMenu.Enable(menu_COMBINE_NODES, 0)
				self.nodeMenu.Enable(menu_CONNECT, 0)
			self.newSpuChoice.Enable(1)
		else:
			self.editMenu.Enable(wxID_CUT, 0)
			self.editMenu.Enable(wxID_COPY, 0)
			self.nodeMenu.Enable(menu_DELETE_NODE, 0)
			self.nodeMenu.Enable(menu_CONNECT, 0)
			self.nodeMenu.Enable(menu_DISCONNECT, 0)
			self.nodeMenu.Enable(menu_SET_HOST, 0)
			self.nodeMenu.Enable(menu_SET_COUNT, 0)
			self.nodeMenu.Enable(menu_SPLIT_NODES, 0)
			self.nodeMenu.Enable(menu_COMBINE_NODES, 0)
			self.newSpuChoice.Enable(0)
		if len(NodeClipboard) > 0:
			self.editMenu.Enable(wxID_PASTE, 1)
		else:
			self.editMenu.Enable(wxID_PASTE, 0)

		# Node menu / servers
		if self.mothership.NumSelectedServers() > 0:
			self.nodeMenu.Enable(menu_SERVER_OPTIONS, 1)
			self.nodeMenu.Enable(menu_SERVER_TILES, 1)
		else:
			self.nodeMenu.Enable(menu_SERVER_OPTIONS, 0)
			self.nodeMenu.Enable(menu_SERVER_TILES, 0)
		if self.mothership.NumSelectedAppNodes() > 0:
			self.nodeMenu.Enable(menu_APP_OPTIONS, 1)
		else:
			self.nodeMenu.Enable(menu_APP_OPTIONS, 0)
			
		if len(self.mothership.Nodes()) > 0:
			self.nodeMenu.Enable(menu_SELECT_ALL_NODES, 1)
			self.nodeMenu.Enable(menu_DESELECT_ALL_NODES, 1)
			self.nodeMenu.Enable(menu_LAYOUT_NODES, 1)
			self.spuMenu.Enable(menu_SELECT_ALL_SPUS, 1)
			self.spuMenu.Enable(menu_SELECT_ALL_SPUS_TYPE, 1)
		else:
			self.nodeMenu.Enable(menu_SELECT_ALL_NODES, 0)
			self.nodeMenu.Enable(menu_DESELECT_ALL_NODES, 0)
			self.nodeMenu.Enable(menu_LAYOUT_NODES, 0)
			self.spuMenu.Enable(menu_SELECT_ALL_SPUS, 0)
			self.spuMenu.Enable(menu_SELECT_ALL_SPUS_TYPE, 0)
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
		#if len(self.undoStack) > 0:
		#	self.editMenu.Enable(menu_UNDO, 1)
		#else:
		#	self.editMenu.Enable(menu_UNDO, 0)
		#if len(self.redoStack) > 0:
		#	self.editMenu.Enable(menu_REDO, 1)
		#else:
		#	self.editMenu.Enable(menu_REDO, 0)
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
								 caption="Hey!",
								 style=wxOK|wxCENTRE|wxICON_EXCLAMATION)
		dialog.ShowModal()


	#----------------------------------------------------------------------
	# File I/O

	def loadConfig(self, fileName):
		"""Load a graph from the named file.
		Return 1 for success, 0 for error."""
		f = open(fileName, "r")
		result = 0
		if f:
			# read first line to check for template
			l = f.readline()
			if not l:
				return
			v = re.search("^TEMPLATE = \"?([^\"]*)\"?", l)
			if v:
				# We're reading a configuration made with a template
				templateName = v.group(1)
				t = InstantiateTemplate(templateName)
				if t.Read(self.mothership, f):
					self.mothership.SetTemplateType(t)
					result = 1
				else:
					self.mothership.SetTemplateType(None)
					result = configio.ReadConfig(self.mothership, f, fileName)
			else:
				# A generic (non-template) configuration
				result = configio.ReadConfig(self.mothership, f, fileName)
			f.close()
			if result:
				self.dirty = false
				self.fileName = fileName
		else:
			self.Notify("Problem opening " + fileName)
		self.UpdateMenus()
		self.drawArea.Refresh()
		return result

	def saveConfig(self):
		"""Save the Chromium configuration to a file."""
		assert self.fileName != None
		# XXX may need to catch exceptions for open()
		result = 0
		f = open(self.fileName, "w")
		if f:
			template = self.mothership.GetTemplateType()
			if (template and template.Validate(self.mothership)):
				# write as templatized config
				result = template.Write(self.mothership, f)
			else:
				# write as generic config
				result = configio.WriteConfig(self.mothership, f)
			f.close()
		if result:
			self.dirty = false
		else:
			response = wxMessageBox("Error writing %s." % self.fileName,
									"Error", wxOK, self)


	def askIfUserWantsToSave(self, action):
		""" Give the user the opportunity to save the current document.
			'action' is a string describing the action about to be taken.  If
			the user wants to save the document, it is saved immediately.  If
			the user cancels, we return false.
		"""
		if not self.dirty:
			return true # Nothing to do.

		if self.fileName:
			name = self.fileName
		else:
			name = "(untitled)"
		response = wxMessageBox("Save changes to " + name + " before " + action + "?",
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


if __name__ == "__main__":
	print "Run configtool.py instead, please."

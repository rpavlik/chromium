# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul


"""The TileDialog class is a dialog used to edit a list of tiles for
a server/network node.  If the server node is an N-instance node the
dialog will display a spin control [1 .. N] to edit the tile list for
any of the N instances.
"""

from wxPython.wx import *
from wxPython.gizmos import *
import crutils


class TileDialog(wxDialog):
	def __init__(self, parent, id, title, numLists, hosts=[""], message=""):
		"""parent, id, and title are the standard wxDialog parameters.
		"""
		assert numLists >= 1
		wxDialog.__init__(self, parent, id, title, pos=wxPoint(-1,-1),
						  style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
		
		id_OK = 1
		id_CANCEL = 2
		id_INSTANCE = 3

		outerSizer = wxBoxSizer(wxVERTICAL)

		if numLists > 1:
			# spin box to choose node instance
			box = wxStaticBox(parent=self, id=-1, label="Node Instance")
			innerSizer = wxStaticBoxSizer(box, wxHORIZONTAL)
			outerSizer.Add(innerSizer, option=0, flag=wxGROW|wxALL, border=4)
			label = wxStaticText(parent=self, id=-1, label="Instance:")
			innerSizer.Add(label, flag=wxALIGN_CENTRE_VERTICAL|wxALL, border=2)
			self.instanceCtrl = wxSpinCtrl(parent=self, id=id_INSTANCE,
										   size=wxSize(50,25),
										   min=1, max=numLists, value="1")
			EVT_SPINCTRL(self.instanceCtrl, id_INSTANCE, self._onInstance)
			self.hostLabel = wxStaticText(parent=self, id=-1,
										  label="Hostname: %s" % hosts[0])
			innerSizer.Add(self.instanceCtrl,
						   flag=wxALIGN_CENTRE_VERTICAL|wxALL, border=2)
			innerSizer.Add(self.hostLabel, flag=wxALIGN_CENTRE_VERTICAL|wxALL,
						   border=6)


		# editable list of tile tuples
		box = wxStaticBox(parent=self, id=-1, label="Edit Tile List")
		innerSizer = wxStaticBoxSizer(box, wxVERTICAL)
		outerSizer.Add(innerSizer, option=1, flag=wxALL|wxGROW, border=4)
		self.listBox = wxEditableListBox(parent=self, id=-1,
										 label="Tiles (x, y, width, height)",
										 size=(300, 200))
		innerSizer.Add(self.listBox, option=1, flag=wxGROW|wxALL, border=2)

		# OK / Cancel buttons
		rowSizer = wxGridSizer(rows=1, cols=2, vgap=4, hgap=20)
		self.OkButton = wxButton(parent=self, id=id_OK, label="OK")
		rowSizer.Add(self.OkButton, option=0, flag=wxALIGN_CENTER)
		self.CancelButton = wxButton(parent=self, id=id_CANCEL, label="Cancel")
		rowSizer.Add(self.CancelButton, option=0, flag=wxALIGN_CENTER)
		outerSizer.Add(rowSizer, option=0, flag=wxGROW|wxALL, border=4)
		EVT_BUTTON(self.OkButton, id_OK, self._onOK)
		EVT_BUTTON(self.CancelButton, id_CANCEL, self._onCancel)

		min = outerSizer.GetMinSize()
		self.SetSizer(outerSizer)
		self.SetAutoLayout(true)
		self.SetSizeHints(minW=min[0], minH=min[1])
		self.SetSize(min)

		self.TileListList = []   # array [numLists] of array of (x, y, w, h)
		self.NumLists = numLists
		for i in range(numLists):
			self.TileListList.append( [] )

		self.OldInstance = 1
		self.Hosts = hosts


	def __LoadWidget(self, i):
		"""Load the widget with the ith tile list."""
		strings = []
		if i < len(self.TileListList):
			for tile in self.TileListList[i]:
				tileString = "(%d, %d, %d, %d)" % tile
				strings.append(tileString)
		self.listBox.SetStrings(strings)
		
	def __ReadWidget(self, i):
		"""Get the strings from the listBox and update the ith tile list."""
		assert i >= 0
		assert i < self.NumLists
		strings = self.listBox.GetStrings()
		tiles = []
		for s in strings:
			# parse "(x,y,w,h)" to get tuple (x,y,w,h)
			# XXX probably need an exception handler
			tile = eval(s)
			if tile and len(tile) == 4:
				tiles.append(tile)
		self.TileListList[i] = tiles
		
	def _onInstance(self, event):
		"""Called when the instance spin control changes."""
		self.__ReadWidget(self.OldInstance - 1)
		i = self.instanceCtrl.GetValue()
		assert i >= 1
		self.__LoadWidget(i - 1)
		if i - 1 < len(self.Hosts):
			self.hostLabel.SetLabel("Hostname: %s" % self.Hosts[i - 1])
		else:
			# use last hostname
			self.hostLabel.SetLabel("Hostname: %s" % self.Hosts[-1])
		self.OldInstance = i
		
	def _onOK(self, event):
		"""Called by OK button"""
		self.EndModal(wxID_OK)

	def _onCancel(self, event):
		"""Called by Cancel button"""
		self.EndModal(wxID_CANCEL)

	def SetTileLists(self, tiles):
		"""Specify list of list of tiles (x,y,w,h) to edit."""
		self.TileListList = tiles
		while len(self.TileListList) < self.NumLists:
			self.TileListList.append( [] )
		self.__LoadWidget(0)
		if self.NumLists > 1:
			self.instanceCtrl.SetValue(1)

	def GetTileLists(self):
		"""Return list of list of tiles (x,y,w,h)."""
		if self.NumLists > 1:
			i = self.instanceCtrl.GetValue() - 1
		else:
			i = 0
		self.__ReadWidget(i)
		return self.TileListList


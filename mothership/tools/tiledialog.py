# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul


"""The TileDialog class is a dialog used to edit a list of tiles for
a server/network node.
"""

from wxPython.wx import *
from wxPython.gizmos import *
import crutils


class TileDialog(wxDialog):
	def __init__(self, parent, id, title, message=""):
		"""parent, id, and title are the standard wxDialog parameters.
		"""
		wxDialog.__init__(self, parent, id, title, pos=wxPoint(-1,-1),
						  style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
		
		id_OK = 1
		id_CANCEL = 2
		id_PATTERN = 3
		id_START = 4
		id_COUNT = 5
		id_GENERATE = 6

		outerSizer = wxBoxSizer(wxVERTICAL)

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

	def _onOK(self, event):
		"""Called by OK button"""
		self.EndModal(wxID_OK)

	def _onCancel(self, event):
		"""Called by Cancel button"""
		self.EndModal(wxID_CANCEL)

	def SetTiles(self, tiles):
		"""Specify list tiles (x,y,w,h) to edit."""
		strings = []
		for tile in tiles:
			tileString = "(%d, %d, %d, %d)" % tile
			strings.append(tileString)
		self.listBox.SetStrings(strings)

	def GetTiles(self):
		"""Return lists of tiles (x,y,w,h)."""
		strings = self.listBox.GetStrings()
		tiles = []
		for s in strings:
			# parse "(x,y,w,h)" to get tuple (x,y,w,h)
			# XXX probably need an exception handler
			tile = eval(s)
			if tile and len(tile) == 4:
				tiles.append(tile)
		return tiles


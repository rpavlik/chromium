# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul


"""The HostDialog class is a dialog used to specify a list of hostnames.
A sequence of hostnames can be defined using a pattern string, start
index and count.  Or, hostnames can be manually specified one-by-one.
"""

from wxPython.wx import *
from wxPython.gizmos import *
import crutils


class HostDialog(wxDialog):
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

		# Hostname generator controls
		if message != "":
			msgText = wxStaticText(parent=self, id=-1, label=message)
			outerSizer.Add(msgText, flag=wxALIGN_CENTRE_HORIZONTAL|wxALL,
						   border=8)

		box = wxStaticBox(parent=self, id=-1, label="Generate Host Names")
		innerSizer = wxStaticBoxSizer(box, wxHORIZONTAL)
		outerSizer.Add(innerSizer, option=0, flag=wxGROW|wxALL, border=4)
		label = wxStaticText(parent=self, id=-1, label="Name pattern:")
		innerSizer.Add(label, flag=wxALIGN_CENTRE_VERTICAL|wxALL, border=2)
		self.patternCtrl = wxTextCtrl(parent=self, id=id_PATTERN,
									  size=wxSize(100,25), value="host#")
		innerSizer.Add(self.patternCtrl,
					   flag=wxALIGN_CENTRE_VERTICAL|wxALL|wxGROW, border=2)
		label = wxStaticText(parent=self, id=-1, label="First index:")
		innerSizer.Add(label, flag=wxALIGN_CENTRE_VERTICAL|wxALL, border=2)
		self.startCtrl = wxSpinCtrl(parent=self, id=id_START,
									size=wxSize(50,25), min=0, initial=1)
		innerSizer.Add(self.startCtrl,
					   flag=wxALIGN_CENTRE_VERTICAL|wxALL, border=2)
		label = wxStaticText(parent=self, id=-1, label="Count:")
		innerSizer.Add(label, flag=wxALIGN_CENTRE_VERTICAL|wxALL, border=2)
		self.countCtrl = wxSpinCtrl(parent=self, id=id_COUNT,
									size=wxSize(50,25), min=0, initial=1)
		innerSizer.Add(self.countCtrl,
					   flag=wxALIGN_CENTRE_VERTICAL|wxALL, border=2)
		self.generateButton = wxButton(parent=self, id=id_GENERATE,
									   label="Generate")
		innerSizer.Add(self.generateButton,
					   flag=wxALIGN_CENTRE_VERTICAL|wxALL, border=2)
		EVT_BUTTON(self.generateButton, id_GENERATE, self._onGenerate)

		# editable list of host names
		box = wxStaticBox(parent=self, id=-1, label="Edit Host Names")
		innerSizer = wxStaticBoxSizer(box, wxVERTICAL)
		outerSizer.Add(innerSizer, option=1, flag=wxALL|wxGROW, border=4)
		self.listBox = wxEditableListBox(parent=self, id=-1,
										 label="Host names", size=(250, 250))
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

	def _onGenerate(self, event):
		"""Called when Generate button is pressed."""
		self.GenerateHosts()

	def _onOK(self, event):
		"""Called by OK button"""
		self.EndModal(wxID_OK)

	def _onCancel(self, event):
		"""Called by Cancel button"""
		self.EndModal(wxID_CANCEL)

	# Override the wxDialog.ShowModal() method
	def ShowModal(self):
		"""Display dialog and return wxID_OK or wxID_CANCEL."""
		retVal = wxDialog.ShowModal(self)
		return retVal

	def SetHosts(self, strings):
		"""Specify a list of hostnames"""
		self.listBox.SetStrings(strings)

	def GetHosts(self):
		"""Return lists of hostnames"""
		return self.listBox.GetStrings()

	def SetHostPattern(self, patternTuple):
		"""Set the naming pattern string"""
		self.patternCtrl.SetValue(patternTuple[0])
		self.startCtrl.SetValue(patternTuple[1])

	def GetHostPattern(self):
		"""Get the naming pattern tuple (string, startIndex)."""
		return (self.patternCtrl.GetValue(), int(self.startCtrl.GetValue()))

	def SetCount(self, count):
		"""Set the host name count"""
		self.countCtrl.SetValue(count)

	def GetCount(self):
		"""Get the host name count"""
		return self.countCtrl.GetValue()

	def GenerateHosts(self):
		"""Generate the host names"""
		pattern = self.patternCtrl.GetValue()
		start = self.startCtrl.GetValue()
		count = self.countCtrl.GetValue()
		names = crutils.MakeHostnames(pattern, start, count)
		self.listBox.SetStrings(names)
		return names



# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul


"""The SPUDialog class builds a dialog with a set of options specified by
the caller.  Options can be boolean, integer, float or string.
"""

from wxPython.wx import *


class SPUDialog(wxDialog):
	def __init__(self, parent, id, title, options=None):
		"""parent, id, and title are the standard wxDialog parameters.
		options is a list of tuples (name, type, count, default, description)
		that describes the controls to put in the dialog.
		"""
		wxDialog.__init__(self, parent, id, title, pos=wxPoint(-1,-1),
						  style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
		
		id_OK = 1
		id_CANCEL = 2

		self._Controls = {}

		outerSizer = wxBoxSizer(wxVERTICAL)

		box = wxStaticBox(parent=self, id=-1, label=title)
		innerSizer = wxStaticBoxSizer(box, wxVERTICAL)
		outerSizer.Add(innerSizer, option=1, flag=wxALL|wxGROW, border=10)
		
		if options:
			i = 0
			for (name, type, count, default, description) in options:
				if type == "bool":
					labString = description + " (" + name + ")"
					ctrl = wxCheckBox(parent=self, id=100+i, label=labString)
					innerSizer.Add(ctrl, flag=wxLEFT|wxRIGHT, border=4)
					ctrl.SetValue( default )
				elif type == "int":
					rowSizer = wxBoxSizer(wxHORIZONTAL)
					labString = description + ": "
					label = wxStaticText(parent=self, id=-1, label=labString)
					rowSizer.Add(label, flag=wxALIGN_CENTRE_VERTICAL)
					for j in range(0, count):
						ctrl = wxSpinCtrl(parent=self, id=100+i, value=str(default),
										  max=2048*2048)
						rowSizer.Add(ctrl)
						i += 1
					innerSizer.Add(rowSizer, flag=wxALL, border=4)
				elif type == "float":
					rowSizer = wxBoxSizer(wxHORIZONTAL)
					labString = description + ": "
					label = wxStaticText(parent=self, id=-1, label=labString)
					rowSizer.Add(label, flag=wxALIGN_CENTRE_VERTICAL)
					for j in range(0, count):
						ctrl = wxTextCtrl(parent=self, id=100+i, value=str(default))
						rowSizer.Add(ctrl)
						i += 1
					innerSizer.Add(rowSizer, flag=wxALL, border=4)
				elif type == "string":
					rowSizer = wxBoxSizer(wxHORIZONTAL)
					labString = description + ": "
					label = wxStaticText(parent=self, id=-1, label=labString)
					rowSizer.Add(label, flag=wxALIGN_CENTRE_VERTICAL)
					ctrl = wxTextCtrl(parent=self, id=100+i, value=default)
					rowSizer.Add(ctrl, option=1, flag=wxEXPAND)
					innerSizer.Add(rowSizer, flag=wxALL|wxEXPAND, border=4)

				# Save this option
				value = default
				self._Controls[name] = ctrl

				i += 1

		# XXX still need to write the callbacks for each control

		rowSizer = wxGridSizer(rows=1, cols=2, vgap=4, hgap=20)
		self.OkButton = wxButton(parent=self, id=id_OK, label="OK")
		rowSizer.Add(self.OkButton, option=0, flag=wxALIGN_CENTER, border=10)
		self.CancelButton = wxButton(parent=self, id=id_CANCEL, label="Cancel")
		rowSizer.Add(self.CancelButton, option=0, flag=wxALIGN_CENTER, border=10)
		outerSizer.Add(rowSizer, option=0, flag=wxGROW|wxBOTTOM, border=10)
		EVT_BUTTON(self.OkButton, id_OK, self._onOK)
		EVT_BUTTON(self.CancelButton, id_CANCEL, self._onCancel)

		min = outerSizer.GetMinSize()
		min[0] = 500
		self.SetSizer(outerSizer)
		self.SetAutoLayout(true)
		self.SetSizeHints(minW=min[0], minH=min[1])
		self.SetSize(min)

	def _onOK(self, event):
		"""Called by OK button"""
		self.EndModal(1)

	def _onCancel(self, event):
		"""Called by Cancel button"""
		self.EndModal(0)

	def IsOption(self, name):
		"""Return true if name is a recognized option."""
		return name in self._Controls

	# name is an SPU option like bbox_line_width
	def SetValue(self, name, newValue):
		"""Set a control's value"""
		assert name in self._Controls
		ctrl = self._Controls[name]
		if isinstance(ctrl, wxSpinCtrl) or isinstance(ctrl, wxCheckBox):
			newValue = int(newValue)
		ctrl.SetValue(newValue)

	# name is an SPU option like bbox_line_width
	def GetValue(self, name):
		"""Return current value of the named control"""
		assert name in self._Controls
		ctrl = self._Controls[name]
		return ctrl.GetValue()

	# Override the wxDialog.ShowModal() method
	def ShowModal(self):
		"""Display dialog and return 0 for cancel, 1 for OK."""
		# Save starting values
		values = {}
		for name in self._Controls:
			values[name] = self._Controls[name].GetValue()
		# Show the dialog
		retVal = wxDialog.ShowModal(self)
		if retVal == 0:
			# Cancelled, restore original values
			for name in values:
				self._Controls[name].SetValue(values[name])
		return retVal

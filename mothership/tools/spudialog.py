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

import string
from wxPython.wx import *


def _BackslashChars(s):
	"""Return the string with tab characters replaced by \t, etc."""
	# XXX do same thing for newlines, etc?
	return string.replace(s, "\t", "\\t")

def _UnBackslashChars(s):
	"""Return the string with \t sequences converted to real tabs, etc."""
	# XXX do same thing for newlines, etc?
	return string.replace(s, "\\t", "\t")


class SPUDialog(wxDialog):
	def __init__(self, parent, id, title, optionList=None):
		"""parent, id, and title are the standard wxDialog parameters.
		optionList is an OptionList object that describes the controls to put
		in the dialog.
		"""
		wxDialog.__init__(self, parent, id, title, pos=wxPoint(-1,-1),
						  style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
		
		id_OK = 1
		id_CANCEL = 2
		id_RESTORE = 3

		self.__Controls = {}
		self.__OptionList = optionList

		outerSizer = wxBoxSizer(wxVERTICAL)

		box = wxStaticBox(parent=self, id=-1, label=title)
		innerSizer = wxStaticBoxSizer(box, wxVERTICAL)
		outerSizer.Add(innerSizer, option=1, flag=wxALL|wxGROW, border=10)
		
		if optionList and len(optionList) > 0:
			i = 0
			for opt in optionList:
				controls = []
				if opt.Type == "BOOL":
					#labString = opt.Description + " (" + opt.Name + ")"
					labString = opt.Description
					# XXX support [opt.Count] widgets?
					assert opt.Count == 1
					ctrl = wxCheckBox(parent=self, id=100+i, label=labString)
					innerSizer.Add(ctrl, flag=wxLEFT|wxRIGHT, border=4)
					ctrl.SetValue( opt.Value[0] )
					controls.append(ctrl)
				elif opt.Type == "INT":
					rowSizer = wxBoxSizer(wxHORIZONTAL)
					labString = opt.Description + ": "
					label = wxStaticText(parent=self, id=-1, label=labString)
					rowSizer.Add(label, flag=wxALIGN_CENTRE_VERTICAL)
					for j in range(0, opt.Count):
						if len(opt.Mins) > j:
							minValue = opt.Mins[j]
						else:
							minValue = 0
						if len(opt.Maxs) > j:
							maxValue = opt.Maxs[j]
						else:
							maxValue = 1000 * 1000 * 1000 # infinity
						if opt.Count <= 2:
							width = 90
						else:
							width = 60
						ctrl = wxSpinCtrl(parent=self, id=100+i,
										  size=wxSize(width,25),
										  value=str(opt.Value[j]),
										  min=minValue, max=maxValue)
						rowSizer.Add(ctrl)
						controls.append(ctrl)
						i += 1
					innerSizer.Add(rowSizer, flag=wxALL, border=4)
				elif opt.Type == "FLOAT":
					rowSizer = wxBoxSizer(wxHORIZONTAL)
					labString = opt.Description + ": "
					label = wxStaticText(parent=self, id=-1, label=labString)
					rowSizer.Add(label, flag=wxALIGN_CENTRE_VERTICAL)
					for j in range(0, opt.Count):
						ctrl = wxTextCtrl(parent=self, id=100+i,
										  size=wxSize(70,25),
										  value=str(opt.Value[j]))
						rowSizer.Add(ctrl)
						controls.append(ctrl)
						i += 1
					innerSizer.Add(rowSizer, flag=wxALL, border=4)
				elif opt.Type == "STRING":
					rowSizer = wxBoxSizer(wxHORIZONTAL)
					labString = opt.Description + ": "
					label = wxStaticText(parent=self, id=-1, label=labString)
					rowSizer.Add(label, flag=wxALIGN_CENTRE_VERTICAL)
					# XXX support [opt.Count] widgets?
					assert opt.Count == 1
					s = _BackslashChars(opt.Value[0])
					ctrl = wxTextCtrl(parent=self, id=100+i, value=s)
					rowSizer.Add(ctrl, option=1, flag=wxEXPAND)
					innerSizer.Add(rowSizer, flag=wxALL|wxEXPAND, border=4)
					controls.append(ctrl)
				elif opt.Type == "ENUM":
					assert opt.Count == 1
					rowSizer = wxBoxSizer(wxHORIZONTAL)
					labString = opt.Description + ": "
					label = wxStaticText(parent=self, id=-1, label=labString)
					rowSizer.Add(label, flag=wxALIGN_CENTRE_VERTICAL)
					ctrl = wxChoice(parent=self, id=-1, choices = opt.Mins)
					ctrl.SetStringSelection(opt.Value[0])
					rowSizer.Add(ctrl, flag=wxALIGN_CENTRE_VERTICAL)
					controls.append(ctrl)
					innerSizer.Add(rowSizer, flag=wxALL|wxEXPAND, border=4)
				else:
					assert opt.Type == "LABEL"
					# just label text
					label = wxStaticText(parent=self, id=-1,
										 label=opt.Description)
					innerSizer.Add(label, flag=wxALL|wxEXPAND, border=4)

				# Save this option
#				value = opt.Value  # a vector
				self.__Controls[opt.Name] = controls   # a vector

				i += 1

		else:
			# no options, display a message
			label = wxStaticText(parent=self, id=-1,
								 label="No options for this SPU class.")
			innerSizer.Add(label, flag=wxALIGN_CENTER|wxALL, border=4)

		rowSizer = wxGridSizer(rows=1, cols=3, vgap=4, hgap=20)
		self.RestoreButton = wxButton(parent=self, id=id_RESTORE,
									  label=" Restore Defaults ")
		rowSizer.Add(self.RestoreButton, option=0, flag=wxALIGN_CENTER)
		self.OkButton = wxButton(parent=self, id=id_OK, label="OK")
		rowSizer.Add(self.OkButton, option=0, flag=wxALIGN_CENTER)
		self.CancelButton = wxButton(parent=self, id=id_CANCEL, label="Cancel")
		rowSizer.Add(self.CancelButton, option=0, flag=wxALIGN_CENTER)
		outerSizer.Add(rowSizer, option=0, flag=wxGROW|wxBOTTOM, border=10)
		EVT_BUTTON(self.RestoreButton, id_RESTORE, self._onRestore)
		EVT_BUTTON(self.OkButton, id_OK, self._onOK)
		EVT_BUTTON(self.CancelButton, id_CANCEL, self._onCancel)

		min = outerSizer.GetMinSize()
		min[0] = 500
		self.SetSizer(outerSizer)
		self.SetAutoLayout(true)
		self.SetSizeHints(minW=min[0], minH=min[1])
		self.SetSize(min)

	def _onRestore(self, event):
		"""Called by Restore Defaults button"""
		# Set all control/widget values to default values
		for opt in self.__OptionList:
			self.SetValue(opt.Name, opt.Default)

	def _onOK(self, event):
		"""Called by OK button"""
		self.EndModal(wxID_OK)

	def _onCancel(self, event):
		"""Called by Cancel button"""
		self.EndModal(wxID_CANCEL)

	def IsOption(self, name):
		"""Return true if name is a recognized option."""
		return name in self.__Controls

	# name is an SPU option like bbox_line_width
	def SetValue(self, name, newValue):
		"""Set a control's value (a list of values)"""
		assert name in self.__Controls.keys()
		ctrls = self.__Controls[name]
		if ctrls:
			count = len(ctrls)
			#print "name = %s  newValue = %s" % (name, str(newValue))
			if len(newValue) != count:
				print "bad newValue %s = --%s--" % (name, newValue)
				print "len = %d  count = %d" % (len(newValue), count)
			assert len(newValue) == count
			if (isinstance(ctrls[0], wxSpinCtrl) or
				isinstance(ctrls[0], wxCheckBox)):
				for i in range(count):
					ival = int(newValue[i])
					ctrls[i].SetValue(ival)
			elif isinstance(ctrls[0], wxChoice):
				assert len(ctrls) == 1  # ENUM limitation, for now
				ctrls[0].SetStringSelection( newValue[0] )
			else:
				# must be (a) text or float box(es)
				assert isinstance(ctrls[0], wxTextCtrl)
				for i in range(count):
					s = _BackslashChars(str(newValue[i]))
					ctrls[i].SetValue(s)

	# name is an SPU option like bbox_line_width
	def GetValue(self, name):
		"""Return current value (a list) of the named control"""
		assert name in self.__Controls.keys()
		ctrls = self.__Controls[name]
		type = self.__OptionList.GetType(name)
		if ctrls:
			result = []
			count = len(ctrls)
			for i in range(count):
				if type == "ENUM":
					result.append(ctrls[i].GetStringSelection())
				elif type == "STRING":
					s = ctrls[i].GetValue()
					s = _UnBackslashChars(s)
					result.append(s)
				elif type == "FLOAT":
					result.append(float(ctrls[i].GetValue()))
				else:
					assert type == "INT" or type == "BOOL"
					result.append(int(ctrls[i].GetValue()))
			return result
		else:
			return [] # empty list

	# Override the wxDialog.ShowModal() method
	def ShowModal(self):
		"""Display dialog and return wxID_OK or wxID_CANCEL."""
		retVal = wxDialog.ShowModal(self)
		return retVal

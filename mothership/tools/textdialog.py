# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul


"""The TextDialog class defines a dialog window for displaying bulk text."""

from wxPython.wx import *
from wxPython.html import *


class TextDialog(wxDialog):
	"""Dialog window for showing html text"""
	def __init__(self, parent, id, title="Help"):
		id_BACK  = 17000
		id_CLOSE = 17001
		wxDialog.__init__(self, parent, id, title, pos=wxPoint(-1,-1),
						  style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)

		outerSizer = wxBoxSizer(wxVERTICAL)

		p = parent
		self.html = wxHtmlWindow(parent=self, id=-1)

		outerSizer.Add(self.html, 1, wxGROW)


		rowSizer = wxGridSizer(rows=1, cols=2, vgap=4)
		self.BackButton = wxButton(parent=self, id=id_BACK, label="Back")
		self.CloseButton = wxButton(parent=self, id=id_CLOSE, label="Close")
		EVT_BUTTON(self.BackButton, id_BACK, self.__onBack)
		EVT_BUTTON(self.CloseButton, id_CLOSE, self.__onClose)
		rowSizer.Add(self.BackButton, 0, wxALIGN_CENTER, 8)
		rowSizer.Add(self.CloseButton, 0, wxALIGN_CENTER, 8)
		outerSizer.Add(rowSizer, 0, wxGROW|wxALL, 8)

		self.SetSizer(outerSizer)
		self.SetAutoLayout(true)
		self.SetSizeHints(minW=720, minH=500)
		self.Centre()

	def LoadPage(self, url):
		"""Load the html window with a document from the given URL"""
		# XXX this isn't a real URL, it can only be a filename
		self.html.LoadPage(url)

	def SetPage(self, html):
		"""Load html into the window"""
		self.html.SetPage(html)

	def __onBack(self, event):
		"""Called by Back button"""
		self.html.HistoryBack()
		# XXX disable back button when we can't go back
#		if self.html.HistoryCanBack():
#			self.BackButton.Enable(TRUE)
#		else:
#			self.BackButton.Enable(FALSE)

	def __onClose(self, event):
		"""Called by Close button"""
		self.Show(FALSE)
		self.EndModal(wxID_OK)



# ======================================================================
# Test routines

class __TestApp(wxApp):
	""" Test harness wxApp class."""

	class __TestFrame(wxFrame):
		def __init__(self, parent, id, title):
			wxFrame.__init__(self, parent, id, title,
					 style = wxDEFAULT_FRAME_STYLE | wxWANTS_CHARS |
						 wxNO_FULL_REPAINT_ON_RESIZE)
			EVT_CLOSE(self, self.doClose)

		def doClose(self, event):
			global app
			self.Destroy()
			app.ExitMainLoop()


	def OnInit(self):
		wxInitAllImageHandlers()

		frame = self.__TestFrame(parent=None, id=-1, title="Test App")
		frame.Show(TRUE)

		dialog = TextDialog(parent=frame, id=-1, title="Help!")
		#dialog.LoadPage("reassembly.html")
		dialog.LoadPage("../../doc/configtool.html")
		dialog.Centre()
		#dialog.ShowModal()
		dialog.Show()
		return TRUE

def _test():
	global app
	app = __TestApp()
	app.MainLoop()
	return

if __name__ == "__main__":
	_test()

	

#!/usr/bin/env python
# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

"""This is the top-level application module for the graphical config tool."""


import os, sys
from wxPython.wx import *
import graph, crutils


#----------------------------------------------------------------------------

class ConfigApp(wxApp):
	""" The main application object.
	"""
	def OnInit(self):
		""" Initialise the application.
		"""
		wxInitAllImageHandlers()

		assert self
		self.DocList = []

		if len(sys.argv) == 1:
			# No file name was specified on the command line -> start with a
			# blank document.
			frame = graph.GraphFrame(None, -1, graph.TitleString)
			#frame.Centre()
			frame.Show(TRUE)
			self.DocList.append(frame)
		else:
			# Load the file(s) specified on the command line.
			for arg in sys.argv[1:]:
				fileName = os.path.join(os.getcwd(), arg)
				if os.path.isfile(fileName):
					title = graph.TitleString + ": " + os.path.basename(fileName)
					frame = graph.GraphFrame(None, -1, title)
					if frame.loadConfig(fileName):
						frame.Show(TRUE)
						self.DocList.append(frame)
					else:
						frame.Destroy()
						return FALSE

		return TRUE

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

def main():
	""" Start up the configuration tool.
	"""
	# Redirect python exceptions to a log file.
	# XXX if we crash upon startup, try commenting-out this next line:
#	sys.stderr = ExceptionHandler()

	# Set the default site file
	#crutils.SetDefaultSiteFile("tungsten.crsite")

	# Find available SPU classes, print the list
	spuClasses = crutils.FindSPUNames()
	print "Found SPU classes: %s" % str(spuClasses)

	# Create and start the application.
	graph.App = ConfigApp(0)
	graph.App.MainLoop()


if __name__ == "__main__":
	main()


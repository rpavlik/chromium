"""Chromium config tool utility functions"""

import re, os, string, sys
sys.path.append("../server")
import crconfig

#----------------------------------------------------------------------

def MakeHostname(format, number):
	"""Return a hostname generated from a format string and number."""
	# find the hash characters first
	p = re.search("#+", format)
	if not p:
		return format
	numHashes = p.end() - p.start()
	numDigits = len(str(number))
	# start building result string
	result = format[0:p.start()]
	# insert padding zeros as needed
	while numHashes > numDigits:
		result += "0"
		numHashes -= 1
	# append the number
	result += str(number)
	# append rest of format string
	result += format[p.end():]
	return result


#----------------------------------------------------------------------

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
		print "making exception handler"
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


#----------------------------------------------------------------------

def FindSPUNamesInDir(spuDirectory):
	"""Return list of SPUs in the given directory."""
	# filenames to match:
	if os.name == "posix":
		matchPattern = "lib([a-zA-Z0-9]+)spu.so"
	elif os.name == "nt":
		matchPattern = "lib([a-zA-Z0-9]+).dll"
	else:
		print "fix me:  unexpected os.name result!"
		abort
	# SPU names to ignore:
	ignorePattern = "error|nop|template|COPY[a-zA-Z0-9]+|passthrough|hlpassthrough|sqpassthrough"
	# scan directory
	files = os.listdir(spuDirectory)
	# build the list of SPUs
	spus = []
	for name in files:
		m = re.match(matchPattern, name)
		if m:
			spuName = m.group(1)
			if not re.match(ignorePattern, spuName):
				spus.append(spuName)
	return spus


def FindSPUNames():
	"""Return turn list of SPUs found in the "default" directory"""
	crlibdir = os.path.join(crconfig.crdir, 'lib', crconfig.arch)
	l = FindSPUNamesInDir(crlibdir)
	return l


#----------------------------------------------------------------------

def GetSPUOptions(spuName):
	"""Use the spuoptions program to get the params/options for the SPU.
	Same result returned as for ParseSPUOptionsFile() above."""
	program = os.path.join(crconfig.crdir, 'bin', crconfig.arch, 'spuoptions')
	command = '%s --pythonmode %s' % (program, spuName)
	f = os.popen(command, 'r')
	if f:
		s = f.read()
		result = eval(s)
		f.close()
		return result
	else:
		print "Error running spuoptions program (where is it?)"
		return 0

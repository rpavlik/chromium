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

def ParseSPUOptionsLine(line):
	"""Parse an SPU option line string and return a tuple:
	(name, description, type, numValues, defaults, mins, maxs)
	"""
	space = '\s+'
	optionPat = '^(option)'
	namePat = '([a-z_]+)'
	descPat = '"([^"]+)"'
	typePat = '(BOOL|INT|FLOAT|STRING)'
	numPat = '([0-9]+)'
	arrayPat = '\[([^\]]*)\]'
	remainderPat = '(.*)$'
	pattern = optionPat + space + namePat + space + descPat + space + typePat + space + numPat + space + arrayPat + remainderPat
	m = re.match(pattern, line)
	if not m:
		print "bad option pattern"
		return 0
	name = m.group(2)
	descrip = m.group(3)
	type = m.group(4)
	num = int(m.group(5))
	default = m.group(6)
	remainder = m.group(7)
	if type == "BOOL":
		defaults = string.split(default, ' ')
		for i in range(num):
			defaults[i] = int(defaults[i])
		mins = []
		maxs = []
	elif type == "INT":
		# extract default values array
		defaults = string.split(default, ' ')
		for i in range(num):
			defaults[i] = int(defaults[i])
		# extract min values array
		m = re.match(space + arrayPat + remainderPat, remainder)
		if m:
			mins = string.split(m.group(1), ' ')
			for i in range(len(mins)):
				mins[i] = int(mins[i])
			remainder = m.group(2)
			# extract max values array
			m = re.match(space + arrayPat +remainderPat, remainder)
			if m:
				maxs = string.split(m.group(1), ' ')
				for i in range(len(maxs)):
					maxs[i] = int(maxs[i])
			else:
				maxs = []
		else:
			mins = []
	elif type == "FLOAT":
		# extract default values array
		defaults = string.split(default, ' ')
		for i in range(num):
			defaults[i] = float(defaults[i])
		# extract min values array
		m = re.match(space + arrayPat + remainderPat, remainder)
		if m:
			mins = string.split(m.group(1), ' ')
			for i in range(len(mins)):
				mins[i] = float(mins[i])
			remainder = m.group(2)
			# extract max values array
			m = re.match(space + arrayPat +remainderPat, remainder)
			if m:
				maxs = string.split(m.group(1), ' ')
				for i in range(len(maxs)):
					maxs[i] = float(maxs[i])
			else:
				maxs = []
		else:
			mins = []
	else:
		assert type == "STRING"
		defaults = []
		defaults.append(default)
		mins = []
		maxs = []
	return (name, descrip, type, num, defaults, mins, maxs)

def ParseSPUOptionsFile(fileHandle):
	"""Parse an SPU options/parameters file and return a two-tuple of the
	SPU parameters and options.  Each being a dictonary."""
	params = {}
	options = []
	assert fileHandle
	while 1:
		line = fileHandle.readline()
		if not line:
			break
		if re.match("^param", line):
			m = re.match("^param\s(\S+)\s(\S+)", line)
			if m:
				param = m.group(1)
				value = m.group(2)
				params[param] = value
			#print "match param %s is %s" % (param, value)
		elif re.match("^option", line):
			opt = ParseSPUOptionsLine(line)
			if opt:
				options.append(opt)
			#print "match options = %s" % str(options)
		else:
			#print "unrecognized line: %s" % line
			pass
	#endwhile
	return (params, options)

def GetSPUOptions(spuName):
	"""Use the spuoptions program to get the params/options for the SPU.
	Same result returned as for ParseSPUOptionsFile() above."""
	program = os.path.join(crconfig.crdir, 'bin', crconfig.arch, 'spuoptions')
	command = '%s %s' % (program, spuName)
	f = os.popen(command, 'r')
	if f:
		result = ParseSPUOptionsFile(f)
		f.close()
		return result
	else:
		print "Error running spuoptions program (where is it?)"
		return 0

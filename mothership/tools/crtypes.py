# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

"""Chromium SPU, Node and Mothership classes used by the config tool.
This may eventually get rolled into the similar classes defined
in the mothership.
"""

# XXX eventually we'll rename these classes to match the mothership.
# That'll allow us to directly read Cr config files and build the
# mothership/node/spu data structures.


import wxPython
import re, string, sys, types
sys.path.append("../server")
import crconfig


# ----------------------------------------------------------------------

class Option:
	"""Class to describe an SPU/node/mothership option."""
	def __init__(self, name, description, type, count, default, mins, maxs):
		assert len(default) == count
		assert len(mins) == count or len(mins) == 0 or type == "ENUM"
		assert len(maxs) == count or len(maxs) == 0
		self.Name = name
		self.Description = description
		self.Type = type  # "BOOL", "INT", "FLOAT", "STRING", "ENUM" or "LABEL"
		self.Count = count
		self.Default = default  # vector[count]
		self.Mins = mins        # vector[count] (except for ENUM)
		self.Maxs = maxs        # vector[count]
		self.Value = default    # vector[count]

	def Clone(self):
		"""Return a new copy of this Option."""
		new = Option(self.Name, self.Description, self.Type, self.Count,
					 self.Default, self.Mins, self.Maxs)
		new.Value = self.Value[:]
		return new

	def Write(self, file, prefixStr="", suffixStr="", subst=[]):
		"""Write the option to the file as
		<prefix>("name" = "value")<suffix>."""
		if self.Count == 1:
			valueStr = str(self.Value[0])
		else:
			valueStr = str(self.Value)
		# do value substitutions (crbindir)
		replaced = 0
		for (old, new) in subst:
			if valueStr == old:
				valueStr = new
				replaced = 1
				break
		# write the option
		if self.Type == "INT" or self.Type == "BOOL":
			file.write(prefixStr + '("' + self.Name + '", ' + valueStr + ')' + suffixStr + '\n')
		elif self.Type == "FLOAT":
			file.write(prefixStr + '("' + self.Name + '", ' + valueStr + ')' + suffixStr + '\n')
		elif self.Type == "STRING":
			# XXX replace " chars with ' chars
			if replaced:
				file.write(prefixStr + '("' + self.Name + '", ' + valueStr + ')' + suffixStr + '\n')
			else:
				file.write(prefixStr + '("' + self.Name + '", "' + valueStr + '")' + suffixStr + '\n')
		elif self.Type == "ENUM":
			file.write(prefixStr + '("' + self.Name + '", "' + valueStr + '")' + suffixStr + '\n')
		else:
			assert self.Type == "LABEL"
			pass

class OptionList:
	"""Container for a group of Option objects."""
	def __init__(self, options=[]):
		self.__Options = []
		for opt in options:
			self.__Options.append(opt)

	def __len__(self):
		"""Return number of options in the list."""
		return len(self.__Options)

	def __getitem__(self, key):
		"""Implements self[key] where key is an integer in [0, len-1]."""
		if key >= 0 and key < len(self.__Options):
			return self.__Options[key]
		else:
			raise IndexError

	def Clone(self):
		"""Return a new copy of this OptionList."""
		newList = []
		for opt in self.__Options:
			newList.append(opt.Clone())
		return OptionList(newList)

	def HasOption(self, optName):
		"""Test if the named option is in this option list."""
		for opt in self.__Options:
			if opt.Name == optName:
				return 1
		return 0

	def GetType(self, optName):
		"""Return data type of named option."""
		for opt in self.__Options:
			if opt.Name == optName:
				return opt.Type
		return None

	def GetCount(self, optName):
		"""Return count (dimension) of named option."""
		for opt in self.__Options:
			if opt.Name == optName:
				return opt.Count
		return 0

	def GetValue(self, optName):
		"""Return value of named option.  Result is a list!"""
		for opt in self.__Options:
			if opt.Name == optName:
				assert len(opt.Value) == opt.Count
				return opt.Value
		return None

	def SetValue(self, optName, value):
		"""Set value of named option.  Value must be a list!"""
		assert type(value) == types.ListType
		for opt in self.__Options:
			if opt.Name == optName:
				assert len(value) == opt.Count
				opt.Value = value
				return

	def Conf(self, name, valueTuple):
		"""Like SetValue(), but take a tuple instead of list."""
		# Get count and type of expected values
		count = self.GetCount(name)
		if count == 0:
			print "Bad config option: %s" % name
			return
		type = self.GetType(name)
		assert type != None

		# values is a tuple, we need to convert it to list.
		# XXX this will be a lot simpler when we finally disallow Conf()
		# from being called with more than 2 parameters.
		valueList = []
		if len(valueTuple) == count:
			# the tuple size matches the expected length
			for i in range(count):
				if type == "INT" or type == "BOOL":
					valueList.append(int(valueTuple[i]))
				elif type == "FLOAT":
					valueList.append(float(valueTuple[i]))
				else:
					valueList.append(valueTuple[i])
		elif len(valueTuple[0]) == count:
			argList = valueTuple[0]
			for i in range(count):
				if type == "INT" or type == "BOOL":
					valueList.append(int(argList[i]))
				elif type == "FLOAT":
					valueList.append(float(argList[i]))
				else:
					valueList.append(argList[i])
		else:
			assert len(valueTuple) == 1
			stringList = string.split(valueString)
			for i in range(count):
				if type == "INT" or type == "BOOL":
					valueList.append(int(stringList[i]))
				elif type == "FLOAT":
					valueList.append(float(stringList[i]))
				else:
					valueList.append(stringList[i])
		self.SetValue(name, valueList)

	def RestoreDefaults(self):
		"""Restore all options to their default values."""
		for opt in self.__Options:
			opt.Value = opt.Default
				
	def Print(self):
		for opt in self.__Options:
			print "%s = %s" % (opt.Name, str(opt.Value))
		
	def Write(self, file, name, substitutions=[]):
		"""Write the list of options to the given file handle."""
		file.write("%s = [\n" % name)
		for opt in self.__Options:
			opt.Write(file, '\t', ',', substitutions)
		file.write("]\n")

	def Read(self, file, substitutions=[]):
		"""Read/parse lines of the form ("name", value) from file until
		we find a line with just a closing brace."""
		while 1:
			line = file.readline()
			if not line:
				return
			if re.match("\s*\]\s*$", line): 
				# a line with just a ]
				return
			if line[-2:] == ",\n":
				line = line[0:-2]  # strip comma and newline
			# do substitutions
			for (old, new) in substitutions:
				line = string.replace(line, old, new)
			(name, value) = eval(line)
			#print "Got option: _%s_ = _%s_" % (name, value)
			assert self.HasOption(name)
			if self.GetCount(name) == 1:
				value = [ value ]
			self.SetValue(name, value)
		return


# ----------------------------------------------------------------------

class SpuObject:
	"""Stream Processing Unit class"""
	# Note that this class doesn't know anything about specific classes
	# of SPUs or their capabilities.  That's a good thing.
	def __init__(self, name, isTerminal=0, maxServers=0):
		self.__Name = name
		self.__IsTerminal = isTerminal
		self.__MaxServers = maxServers
		self.__ServerPort = 7000
		self.__ServerProtocol = "tcpip"
		self.__Servers = []
		self.__OptionList = None
		self.__TileLayoutFunction = None
		# graphics-related
		self.__X = 0
		self.__Y = 0
		self.__Width = 0
		self.__Height = 30
		self.__IsSelected = 0
		self.__OutlinePen = wxPython.wx.wxPen(wxPython.wx.wxColor(0,0,0),
											  width=1, style=0)
		self.__FillBrush = wxPython.wx.wxLIGHT_GREY_BRUSH

	def Clone(self):
		"""Return a deep copy/clone of this SpuObject"""
		newSpu = SpuObject(self.__Name, self.__IsTerminal, self.__MaxServers)
		newSpu.__Name = self.__Name
		newSpu.__ServerPort = self.__ServerPort
		newSpu.__ServerProtocol = self.__ServerProtocol
		newSpu.__IsSelected = self.__IsSelected
		newSpu.__Servers = self.__Servers[:]  # [:] is list copy
		newSpu.__OptionList = self.__OptionList.Clone()
		return newSpu

	def IsTerminal(self):
		"""Return true if this SPU has to be the last in a chain (a terminal)
		"""
		return self.__IsTerminal

	def MaxServers(self):
		"""Return the max number of servers this SPU can connect to."""
		return self.__MaxServers

	def Select(self):
		self.__IsSelected = 1

	def Deselect(self):
		self.__IsSelected = 0

	def IsSelected(self):
		return self.__IsSelected

	def CanAddServer(self):
		"""Test if a server can be added to this SPU.
		Return "OK" if so, else return reason why not.
		"""
		if self.__MaxServers == 0:
			return "This SPU doesn't have a command packer"
		if len(self.__Servers) >= self.__MaxServers:
			return "This SPU is limited to %d server(s)" % self.__MaxServers
		return "OK"

	def AddServer(self, serverNode, protocol='tcpip', port=7000):
		"""Add a server to this SPU.  The SPU must have a packer!"""
		if (not serverNode in self.__Servers
			and len(self.__Servers) < self.__MaxServers):
			self.__Servers.append(serverNode)
			self.__ServerProtocol = protocol
			self.__ServerPort = port
		else:
			print "AddServer() failed!"

	def RemoveServer(self, serverNode):
		if serverNode in self.__Servers:
			self.__Servers.remove(serverNode)

	def RemoveAllServers(self):
		self.__Servers = []

	def GetServers(self):
		"""Return the list of servers for this SPU.
		For a pack SPU the list will contain zero or one server.
		For a tilesort SPU the list will contain zero or more servers.
		Other SPU classes have no servers.
		"""
		return self.__Servers

	def Name(self):
		return self.__Name

	def GetOptions(self):
		"""Return the SPU's options (an OptionList)"""
		return self.__OptionList

	def SetOptions(self, optionlist):
		"""Set the SPU's options (an OptionList)"""
		assert isinstance(optionlist, OptionList)
		self.__OptionList = optionlist

	def GetOption(self, optName):
		"""Return current value of the named SPU option.
		Result is a list!"""
		assert self.__OptionList.HasOption(optName)
		return self.__OptionList.GetValue(optName)
		
	def SetOption(self, optName, value):
		"""Set the value of a particular SPU option.  Value is a list!"""
		assert self.__OptionList.HasOption(optName)
		assert type(value) == types.ListType
		self.__OptionList.SetValue(optName, value)
		
	def Conf(self, var, *values):
		"""Set an option, via config file"""
		# XXX Eventually we'll remove the '*' for varargs!
		self.__OptionList.Conf(var, values)

	def TileLayoutFunction(self, func):
		"""Set the tile layout function for a tilesort SPU"""
		self.__TileLayoutFunction = func
		
	def PrintOptions(self):
		self.__OptionList.Print()

	def SetPosition(self, x, y):
		self.__X = x
		self.__Y = y

	def GetPosition(self):
		return (self.__X, self.__Y)

	def GetWidth(self):
		return self.__Width

	def GetHeight(self):
		return self.__Height

	def PickTest(self, x, y):
		if (x >= self.__X and x < self.__X + self.__Width and
			y >= self.__Y and y < self.__Y + self.__Height):
			return 1
		else:
			return 0

	def Layout(self, dc):
		"""Compute width and height for drawing this SPU"""
		(w, h) = dc.GetTextExtent(self.__Name)
		self.__Width = w + 8
		self.__Height = h + 8
		
	def Draw(self, dc):
		"""Draw this SPU as a simple labeled box"""
		dc.SetBrush(self.__FillBrush)
		if self.__IsSelected:
			self.__OutlinePen.SetWidth(3)
		else:
			self.__OutlinePen.SetWidth(1)
		dc.SetPen(self.__OutlinePen)
		# if width is zero, compute width/height now
		if self.__Width == 0:
			self.Layout(dc)
		# draw the SPU as a rectangle with text label
		dc.DrawRectangle(self.__X, self.__Y, self.__Width, self.__Height)
		dc.DrawText(self.__Name, self.__X + 4, self.__Y + 4)
		if self.__MaxServers > 0:
			# draw the output plug (a little black rect)
			dc.SetBrush(wxPython.wx.wxBLACK_BRUSH)
			dc.DrawRectangle(self.__X + self.__Width,
							 self.__Y + self.__Height/2 - 5, 4, 10)
		elif self.__IsTerminal:
			# draw a thick right edge on the box
			self.__OutlinePen.SetWidth(3)
			dc.SetPen(self.__OutlinePen)
			dc.DrawLine(self.__X + self.__Width, self.__Y + 1,
						self.__X + self.__Width, self.__Y + self.__Height - 2)


# ----------------------------------------------------------------------

class Node:
	"""Base class for the ServerNode and NetworkNode classes.
	Not to be used directly by clients!
	Note: A single Node instance can actually represent N Chromium nodes
	by setting count > 1.  This allows compact, convenient representations
	for may configurations.
	"""

	def __init__(self, hostnames, isServer, count, color):
		self.__Count = count
		self.__X = 0
		self.__Y = 0
		self.__Width = 0
		self.__Height = 0
		self.__SpuChain = []
		self.__IsServer = isServer
		self.__IsSelected = 0
		self.__InputPlugPos = (0,0)
		self.__Color = color
		self.__Brush = wxPython.wx.wxBrush(color)
		#self.__Hosts = hostnames
		#self.__Label = ""
		self.SetHosts(hostnames)
		self.__HostPattern = (hostnames[0], 1)
		self.__SPUdir = ""
		self.__FontHeight = 0
		self.__AutoStart = None


	def Clone(self):
		"""Return a deep copy/clone of this Node object"""
		newNode = Node(self.__Hosts, self.__IsServer, self.__Count,
					   self.__Color)
		for spu in self.SPUChain():
			newSpu = spu.Clone()
			newNode.AddSPU(newSpu)
		newNode.SetPosition(self.__X, self.__Y)
		newNode.__IsSelected = self.__IsSelected
		return newNode

	def IsAppNode(self):
		"""Return true if this is an app node, false otherwise."""
		return not self.__IsServer

	def IsServer(self):
		"""Return true if this is a server node, false otherwise."""
		return self.__IsServer

	def HasAvailablePacker(self):
		"""Return true if we can connect a server to this node."""
		if len(self.__SpuChain) >= 1 and self.LastSPU().CanAddServer() == "OK":
			return 1
		else:
			return 0

	def HasPacker(self):
		"""Return true if the last SPU has a packer."""
		if len(self.__SpuChain) >= 1 and self.LastSPU().MaxServers() > 0:
			return 1
		else:
			return 0

	def HasChild(self, childCandidate):
		"""Test if childCandidate is a down-stream child (server) of this node
		"""
		if self in childCandidate.GetServers():
			return 1
		else:
			for server in childCandidate.GetServers():
				return self.HasChild(server)
			return 0

	def SetHosts(self, namelist):
		"""Specify a list of hostnames for this (N-instance) node."""
		self.__Hosts = namelist
		if self.__IsServer:
			self.__Label = "Server node host=" + namelist[0]
		else:
			self.__Label = "App node host=" + namelist[0]
		self.InvalidateLayout()

	def GetHosts(self):
		"""Return the host name list."""
		return self.__Hosts

	def SetHostNamePattern(self, patternTuple):
		"""Set the host naming pattern tuple (string, startIndex)."""
		self.__HostPattern = patternTuple

	def GetHostNamePattern(self):
		"""Return (pattern, firstIndex) tuple for naming hosts."""
		return self.__HostPattern

	def SetCount(self, count):
		"""Set the host count"""
		assert count >= 1
		self.__Count = count

	def GetCount(self):
		"""Return the host count"""
		return self.__Count

	def Select(self):
		self.__IsSelected = 1

	def Deselect(self):
		self.__IsSelected = 0

	def IsSelected(self):
		return self.__IsSelected

	def NumSPUs(self):
		"""Return number of SPUs in the chain"""
		return len(self.__SpuChain)

	def LastSPU(self):
		"""Return the last SPU in this node's SPU chain"""
		if len(self.__SpuChain) == 0:
			return 0
		else:
			return self.__SpuChain[-1]

	def GetSPU(self, pos):
		"""Return this node's SPU chain"""
		assert pos >= 0
		assert pos < len(self.__SpuChain)
		return self.__SpuChain[pos]

	def SPUChain(self):
		"""Return this node's SPU chain"""
		return self.__SpuChain

	def GetFirstSelectedSPUPos(self):
		"""Return the position (index) of this node's first selected SPU"""
		pos = 0
		for spu in self.__SpuChain:
			if spu.IsSelected():
				return pos
			pos += 1
		return -1

	def AddSPU(self, s, pos = -1):
		"""Add a new SPU at the given position (-1 = the end)"""
		if pos < 0:
			# add at tail
			self.__SpuChain.append(s)
		else:
			# insert at [pos]
			assert pos >= 0
			assert pos <= len(self.__SpuChain)
			self.__SpuChain.insert(pos, s)
		self.InvalidateLayout()

	def RemoveSPU(self, spu):
		"""Remove an SPU from the node's SPU chain"""
		if spu in self.__SpuChain:
			self.__SpuChain.remove(spu)
		else:
			print "Problem spu not in spu chain!"

	def GetServers(self):
		"""Return a list of servers that the last SPU (a packing SPU) are
		connected to."""
		if self.NumSPUs() > 0:
			return self.LastSPU().GetServers()
		else:
			return []

	def RemoveAllServers(self):
		"""Remove all servers from the last SPU (a packing SPU)"""
		if self.NumSPUs() > 0:
			self.LastSPU().RemoveAllServers()

	def IsSimilarTo(self, node):
		"""Test if this node is similar to the given node"""
		# compare node type (app vs server)
		if self.IsServer() != node.IsServer():
			return 0
		# compare SPU chains
		chain1 = self.SPUChain()
		chain2 = node.SPUChain()
		if len(chain1) != len(chain2):
			return 0
		for i in range(len(chain1)):
			if chain1[i].Name() != chain2[i].Name():
				return 0
		return 1  # close enough

	def GetPosition(self):
		"""Return screen position (x, y) for this node icon."""
		return (self.__X, self.__Y)

	def SetPosition(self, x, y):
		"""Set screen position for this node icon."""
		self.__X = x
		self.__Y = y

	def GetInputPlugPos(self):
		"""Return the (x,y) coordinate of the node's input socket."""
		assert self.__IsServer
		return self.__InputPlugPos

	def GetOutputPlugPos(self):
		"""Return the (x,y) coordinate of the node's output socket."""
		assert self.NumSPUs() > 0
		last = self.LastSPU()
		assert last.MaxServers() > 0
		(x, y) = last.GetPosition()
		x += last.GetWidth() + 2
		y += last.GetHeight() / 2
		return (x, y)

	def InvalidateLayout(self):
		"""Signal that this node needs its layout updated."""
		self.__Width = 0

	def Layout(self, dc):
		"""Compute width and height for drawing this node"""
		self.__Width = 5
		for spu in self.__SpuChain:
			spu.Layout(dc)
			self.__Width += spu.GetWidth() + 2
		self.__Width += 8
		(w, h) = dc.GetTextExtent(self.__Label)
		if self.__Width < w + 8:
			self.__Width = w + 8
		if self.__Width < 100:
			self.__Width = 100
		if self.__Height == 0:
			self.__Height = int(h * 3.5)
		if self.__FontHeight == 0:
			(span, self.__FontHeight) = dc.GetTextExtent(self.__Label)

	def Draw(self, dc, dx=0, dy=0):
		"""Draw this node.  (dx,dy) are the temporary translation values
		used when a mouse drag is in progress."""
		# setup the brush and pen
		dc.SetBrush(self.__Brush)
		p = wxPython.wx.wxPen(wxPython.wx.wxColor(0,0,0), width=1, style=0)
		if self.__IsSelected:
			p.SetWidth(3)
		dc.SetPen(p)
		x = self.__X + dx
		y = self.__Y + dy

		if self.__Width == 0 or self.__Height == 0:
			self.Layout(dc)

		# draw the node's box
		if self.__Count > 1:
			# draw the "Nth box"
			dc.DrawRectangle(x + 8, y + self.__FontHeight + 4,
							 self.__Width, self.__Height)
			dc.DrawText(" ... Count = %d" % self.__Count,
						x + 12, y + self.__Height + 1 )
		dc.DrawRectangle(x, y, self.__Width, self.__Height)
		if self.__IsServer:
			dc.DrawText(self.__Label, x + 4, y + 4)
			# draw the unpacker plug
			px = x - 4
			py = y + self.__Height / 2
			self.__InputPlugPos = (px, py)
			dc.SetBrush(wxPython.wx.wxBLACK_BRUSH)
			dc.DrawRectangle(px, py - 5, 4, 10)
		else:
			dc.DrawText(self.__Label, x + 4, y + 4)

		# draw the SPUs
		x = x + 5
		y = y + 20
		for spu in self.__SpuChain:
			spu.SetPosition(x, y)
			spu.Draw(dc)
			x = x + spu.GetWidth() + 2

	def PickTest(self, x, y):
		"""Return 0 if this node is not picked.
		Return 1 if the node was picked, but not an SPU
		Return n if the nth SPU was picked.
		"""
		# try the SPUs first
		i = 0
		for spu in self.__SpuChain:
			if spu.PickTest(x,y):
				return 2 + i
			i = i + 1
		# now try the node itself
		if (x >= self.__X and x < self.__X + self.__Width and
			y >= self.__Y and y < self.__Y + self.__Height):
			return 1
		elif self.__Count > 1:
			# adjust pick coord and test against the "Nth box"
			x -= 8
			y -= self.__FontHeight + 4
			if (x >= self.__X and x < self.__X + self.__Width and
				y >= self.__Y and y < self.__Y + self.__Height):
				return 1
			else:
				return 0
		else:
			return 0

	def SPUDir(self, dir):
		"""Set SPU directory, from config file."""
		self.__SPUdir = dir

	def GetSPUDir(self):
		"""Return SPU directory."""
		return self.__SPUdir

	def ClientDLL(self, dll):
		"""Needed for reading config files."""
		pass

	def AutoStart(self, program):
		"""Needed for reading config files."""
		self.__AutoStart = program
		pass

	def GetAutoStart(self):
		"""Return autostart parameters."""
		return self.__AutoStart

	def SetOption(self, name, value):
		"""Set named option"""
		self._Options.SetValue(name, value)

	def GetOptions(self):
		"""Return the server OptionList."""
		return self._Options

	def Conf(self, name, *values):
		"""Set an option, via config file"""
		# XXX Eventually we'll remove the '*' for varargs!
		self._Options.Conf(name, values)
		

class NetworkNode(Node):
	"""A CRNetworkNode object"""
	def __init__(self, hostnames=["localhost"], count=1):
		Node.__init__(self, hostnames, 1, count,
					  color=wxPython.wx.wxColor(210,105,135))
		# __Tiles is an array[nodeIndex] of arrays of (x,y,w,h) tuples
		self.__Tiles = [ [] ]
		# Server node options, defined just like SPU options
		self._Options = OptionList( [
			Option("optimize_bucket", "Optimized Extent Bucketing", "BOOL", 1, [1], [], []),
			Option("lighting2", "Generate Lightning-2 Strip Headers", "BOOL", 1, [0], [], []),
			Option("only_swap_once", "Only swap once for N clients", "BOOL", 1, [0], [], []),
			Option("spu_dir", "SPU Directory", "STRING", 1, [""], [], [])
			] )

	def Clone(self):
		"""Return a deep copy of this NetworkNode."""
		newNode = NetworkNode(self.GetHosts(), self.GetCount())
		for spu in self.SPUChain():
			newSpu = spu.Clone()
			newNode.AddSPU(newSpu)
		pos = self.GetPosition()
		newNode.SetPosition(pos[0], pos[1])
		if self.IsSelected():
			newNode.Select()
		newNode.SetHostNamePattern( self.GetHostNamePattern() )
		newNode.__Tiles = self.__Tiles[:]
		newNode._Options = self._Options.Clone()
		assert isinstance(newNode, NetworkNode)
		return newNode
		
	def AddTile(self, x, y, width, height, nodeIndex=0):
		"""Add a tile to nth server (for tilesort only)"""
		# lengthen the tiles list if needed
		while nodeIndex > len(self.__Tiles) - 1:
			self.__Tiles.append( [] )
		assert nodeIndex < len(self.__Tiles)
		# save the tile
		self.__Tiles[nodeIndex].append((x, y, width, height))

	def SetTiles(self, tileList, nodeIndex=0):
		"""Set the tile list for the nth server."""
		while nodeIndex > len(self.__Tiles) - 1:
			self.__Tiles.append( [] )
		self.__Tiles[nodeIndex] = tileList

	def GetTiles(self, nodeIndex=0):
		"""Return the nth server's list of tiles."""
		if nodeIndex < len(self.__Tiles):
			return self.__Tiles[nodeIndex]
		else:
			return []

	def DeleteTiles(self):
		"""Delete all tiles on this server."""
		self.__Tiles = [ [] ]

class ApplicationNode(Node):
	"""A CRApplicationNode object"""
	def __init__(self, hostnames=["localhost"], count=1):
		Node.__init__(self, hostnames, 0, count,
					  color=wxPython.wx.wxColor(55,160,55))
		# Application node options, defined just like SPU options
		self._Options = OptionList( [
			Option("command_help",
				   "Program name and arguments.\n" +
				   "   %N will be replaced by the number of application nodes.\n" +
				   "   %I (eye) will be replaced by each application node's index.\n" +
				   "   %0 (zero) will be replaced by the Zeroth argument on the first" +
				   " app node only.\n" +
				   "   Example command: 'psubmit -size %N -rank %I -clear %0'\n" +
				   "   Zeroth arg: '-swap'",
				   "LABEL", 0, [], [], []),
			Option("application", "Command", "STRING", 1, [""], [], []),
			Option("zeroth_arg", "Zeroth arg", "STRING", 1, [""], [], []),
			Option("start_dir", "Start Directory", "STRING", 1, [crconfig.crbindir], [], []),
			Option("client_dll", "Client DLL", "STRING", 1, [""], [], []),
			Option("spu_dir", "SPU Directory", "STRING", 1, [""], [], []),
			Option("minimum_window_size", "Minimum Chromium App Window Size (w h)", "INT", 2, [0, 0], [0, 0], []),
			Option("match_window_title", "Match App Window Title", "STRING", 1, [""], [], []),
			Option("track_window_size", "Track App Window Size Changes", "BOOL", 1, [0], [], []),
			Option("show_cursor", "Show Virtual Cursor", "BOOL", 1, [0], [], []),
			] )

	def Clone(self):
		"""Return a deep copy of this ApplicationNode."""
		newNode = ApplicationNode(self.GetHosts(), self.GetCount())
		for spu in self.SPUChain():
			newSpu = spu.Clone()
			newNode.AddSPU(newSpu)
		pos = self.GetPosition()
		newNode.SetPosition(pos[0], pos[1])
		if self.IsSelected():
			newNode.Select()
		newNode.SetHostNamePattern( self.GetHostNamePattern() )
		assert isinstance(newNode, ApplicationNode)
		return newNode

	def StartDir(self, dir):
		self._Options.SetValue('start_dir', [dir])

	def SetApplication(self, app):
		self._Options.SetValue('application', [app])
		

# ----------------------------------------------------------------------

class Mothership:
	"""The mothership class"""

	def __init__(self):
		self.__Nodes = []
		self.__TemplateType = ""
		self.__TemplateVars = {}
		self.__Options = OptionList( [
			Option("MTU", "Max Transmission Unit (bytes)", "INT", 1, [1024*1024], [0], []),
			Option("auto_start", "Automatically Start Servers", "BOOL", 1, [0], [], []),
			] )

	def GetOptions(self):
		"""Get the mothership/global options (an OptionList)"""
		return self.__Options

	def SetOption(self, name, value):
		"""Set a global option value (value must be a list)"""
		assert self.__Options.HasOption(name)
		self.__Options.SetValue(name, value)

	def Conf(self, paramName, value):
		"""Set a mothership config parameter"""
		# Ugh, this is more complicated than one would expect
		if not self.__Options.HasOption(paramName):
			print "bad name in Conf(): %s" % paramName
			return
		count = self.__Options.GetCount(paramName)
		type = self.__Options.GetType(paramName)

		#print "paramName=%s value = %s" % (paramName, str(value))
		#print "type = %s  count = %d" % (type, count)
		if count == 1:
			if (type == "INT" or type == "BOOL" or type == "FLOAT"):
				valList = [ value ]
			elif type == "STRING":
				if len(value) == 0:
					valList = [ "" ]
				else:
					valList = [ value ]
			else:
				print "PROBLEM: unknown type for %s" % paramName
		else:
			# value should already be a list
			assert len(value) == count
			valList = value
		self.SetOption(paramName, valList)

	def MTU(self, bytes):
		"""Set the MTU size (in bytes)."""
		self.SetOption("MTU", [ bytes ])

	def AllSPUConf(self, spuName, var, *values):
		"""Set var/values for all SPUs that match spuName"""
		# XXX Eventually we'll remove the '*' for varargs!
		# needed for loading configs
		pass

	def Go(self):
		"""Run the mothership."""
		# needed for loading configs
		pass

	def SetTemplateType(self, type):
		"""Set the template type."""
		self.__TemplateType = type

	def GetTemplateType(self):
		"""If this mothership config matches a template, return the template
		name."""
		return self.__TemplateType

	def AddNode(self, node):
		"""Add a node to the mothership."""
		self.__Nodes.append(node)

	def RemoveNode(self, node):
		"""Remove a node from the mothership."""
		if node in self.__Nodes:
			self.__Nodes.remove(node)

	def Nodes(self):
		"""Return reference to the mothership's node list."""
		return self.__Nodes

	def SelectedNodes(self):
		"""Return a list of the selected nodes"""
		selected = []
		for node in self.__Nodes:
			if node.IsSelected():
				selected.append(node)
		return selected

	def SelectAllNodes(self):
		for node in self.__Nodes:
			node.Select()

	def DeselectAllNodes(self):
		for node in self.__Nodes:
			node.Deselect()

	def NumSelectedNodes(self):
		"""Return number of selected nodes."""
		n = 0
		for node in self.__Nodes:
			if node.IsSelected():
				n += 1
		return n

	def NumSelectedServers(self):
		"""Return number of selected server/network nodes."""
		n = 0
		for node in self.__Nodes:
			if node.IsServer() and node.IsSelected():
				n += 1
		return n

	def NumSelectedAppNodes(self):
		"""Return number of selected application nodes."""
		n = 0
		for node in self.__Nodes:
			if node.IsAppNode() and node.IsSelected():
				n += 1
		return n

	def __compareFunc(self, node1, node2):
		(x1, y1) = node1.GetPosition()
		(x2, y2) = node2.GetPosition()
		if x1 < x2:
			return -1
		elif x1 > x2:
			return 1
		else:
			if y1 < y2:
				return -1
			elif y1 > y2:
				return 1
			else:
				return 0

	def SortNodesByPosition(self, list):
		"""Return a list all the nodes sorted by position (X-major)"""
		list.sort(self.__compareFunc)

	def LayoutNodes(self):
		"""Compute reasonable window positions for all the nodes"""
		nodeColumn = {}
		nodeRow = {}
		# first, put all nodes into column 0
		for node in self.__Nodes:
			nodeColumn[node] = 0
		# assign nodes to columns
		# depth-first traversal over the node graph, using a stack
		stack = []
		for node in self.__Nodes:
			stack.append(node)
		while len(stack) > 0:
			# pop node
			node = stack[0]
			stack.remove(node)
			# loop over this node's children
			for server in node.GetServers():
				if server in stack:
					stack.remove(server)
				# position this server to the right of the node
				nodeColumn[server] = nodeColumn[node] + 1
				# push the server's children onto the unresolved list
				if len(server.GetServers()) > 0:
					stack.insert(0, server)

		# compute rows for nodes
		columnSize = {}
		for node in self.__Nodes:
			col = nodeColumn[node]
			if col in columnSize:
				row = columnSize[col]
				columnSize[col] += 1
			else:
				row = 0
				columnSize[col] = 1
			nodeRow[node] = row
		# find tallest column
		tallest = 0
		for col in columnSize.keys():
			if columnSize[col] >= tallest:
				tallest = columnSize[col]
		# set the (x,y) positions for each node
		for node in self.__Nodes:
			col = nodeColumn[node]
			row = nodeRow[node]
			x = col * 200 + 10
			y = 10 + (tallest - columnSize[col]) * 35 + row * 70
			node.SetPosition(x, y)
			node.InvalidateLayout()

	def GetSelectedSPUs(self):
		"""Return a list of all the selected SPUs"""
		spuList = []
		for node in self.__Nodes:
			if node.IsSelected():
				for spu in node.SPUChain():
					if spu.IsSelected():
						spuList.append(spu)
		return spuList

	def NumSelectedSPUs(self):
		"""Return number of selected SPUs"""
		count = 0
		for node in self.__Nodes:
			if node.IsSelected():
				for spu in node.SPUChain():
					if spu.IsSelected():
						count += 1
		return count

	def FindClients(self, serverNode):
		"""Return a list of the client nodes for the given server"""
		clients = []
		for node in self.__Nodes:
			servers = node.GetServers()
			if serverNode in servers:
				clients.append(node)
		return clients


# ----------------------------------------------------------------------

class Assembly:
	"""An assembly represents a specific configuration of nodes and SPUs"""
	def __init__(self, name):
		self.__Name = name


	def Draw(self, dc):
		"""Draw this assembly"""
		pass
	

# ======================================================================
# Test routines

def _test():
	o1 = Option("x", "X value", "FLOAT", 1, [1.2], [-10], [10])
	o2 = Option("y", "Y value", "FLOAT", 1, [1.2], [-10], [10])
	o3 = o2.Clone()

	l = OptionList( [ o1, o2 ] )
	l2 = l.Clone()

	l.SetValue("x", [5])

	l2.Print()

	o2.Value = 4
	print o2.Value
	print o3.Value

if __name__ == "__main__":
	_test()


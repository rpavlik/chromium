# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

"""Chromium mothership functions

These functions are used in configuration files to direct the mothership.
Use these functions to describe your SPU network.  The SPU network is a DAG
of nodes.  Each node contains a SPU chain.  At the top of the graph is an
CRApplicationNode.  It connects to one or more CRNetworkNodes.

Public functions and classes:
    CR:             Main class that controls the mothership
    SPU:            Class that defines a Stream Processing Unit.
    CRNetworkNode:  Sub class of CRNode that defines a node in the SPU
                    graph that handles incoming and outgoing network
                    traffic.
    CRApplicationNode:
                    Sub class of CRNode that defines the start of the the
                    SPU graph.
    CRAddStartupCallback:
                    Add a callback to be called on cr.Go()

Other internal functions/classes:
    CRNode:         Base class that defines a node in the SPU graph
    CRDebug:        Used to print out debugging messages.
    CROutput:       Used to print messages to a logfile.
    Fatal:          Used to print out a debugging messages and exit.
    MakeString:     Converts a Python object to a string.
    SockWrapper:    Internal convenience class for handling sockets
"""

import sys, string, types, traceback, re, threading, os, socket, select, signal, pickle, copy

from crconfig import arch, crdir, crbindir, crlibdir

# This controls whether debug messages are printed (1=yes, 0=no)
DebugMode = 0

# Some help in figuring out the domains of some non-qualified hostnames
__hostPrefixPairs__= [ ('iam','.psc.edu'), ('tg-v','.uc.teragrid.org') ]

def CRInfo( str ):
	"""CRInfo(str)
	Prints informational messages to stderr."""
	print >> sys.stderr, str

def CRDebug( str ):
	"""CRDebug(str)
	Prints debugging message to stderr."""
	if DebugMode:
		print >> sys.stderr, str

def CROutput( str ):
	"""CROutput(str)
	Prints message to logfile."""
	file = os.environ.get("CR_PERF_MOTHERSHIP_LOGFILE")
	if file:
		f = open(file, "a")
		if f:
			f.write("%s\n" % str )
			f.close()
		else:
			CRDebug("Unable to open performance monitoring log file %s\n" % file)
	else:
		CRDebug("NO Performance Logfile set, check CR_PERF_MOTHERSHIP_LOGFILE")

def CRAddStartupCallback( cb ):
	"""CRAddStartupCallback( cb )
	Causes cb(thisCR) to be called from thisCR.Go()."""
	CR.startupCallbacks.append(cb)

allSPUs = {}

def Fatal( str ):
	"""Fatal(str)
	Prints debugging message to stderr and skeddadles."""
	print >> sys.stderr, str
	sys.exit(-1)

def MakeString( x ):
	"""MakeString(x)
	Converts an object to a string"""
	if type(x) == types.StringType:
		return x
	else:
		return repr(x)

def SameHost( host1, host2 ):
	"""Return 1 if host1 and host2 name the same host.	Return 0 otherwise.
	For example, if host1='foo' and host2='foo.bar.com' we'll return 1.
	"""
	try: 
		if socket.gethostbyname(host1) == socket.gethostbyname(host2):
			return 1
		else:
			return 0
	except socket.gaierror:
		if string.split(host1,".")[0] == string.split(host2,".")[0]:
			return 1
		else:
			return 0;

# Return true if the specified hostname fulfills the specified
# constraint.  The constraint is of the form "test:predicate".
# Right now we only implement a single test (a name match), but
# there is room for others.
reportedConstraintWarnings = { }
def ConstraintMatches(constraint, hostname):
	pieces = string.split(constraint, ":", 1)
	test = pieces[0]
	try:
		predicate=pieces[1]
	except:
		predicate=None

	if test == "name":
		# The predicate is a pattern that the hostname should match.
		return re.search(predicate, hostname)
	elif not reportedConstraintWarnings.has_key(test):
		print "***WARNING: Unknown constraint test '%s'" % test
		reportedConstraintWarnings[test] = 1
		return 0
	else:
		return 0

# This structure will contain a list of all dynamic host indicators
# found during definition; they will be assigned as servers come in
# through the MatchNode() routine (following).
dynamicHosts = { }

def MatchNode(node, hostToMatch):

	hostspec = node.host
	
	# If the node is specified dynamically, we'll use its
	# dynamic replacement for its hostname.  Otherwise,
	# we'll use its hostname as specified.
	if hostspec[0:1] == "#":
		# This node is dynamic; it may or may not have already
		# been resolved to a hostname.  See whether it has.
		if dynamicHosts.has_key(hostspec):
			actualHost = dynamicHosts[hostspec]
		else:
			# It hasn't yet been resolved.  See whether
			# there are any other constraints on the match.
			# Any failed constraint means the match fails.
			if node.config.has_key('dynamic_constraints'):
				constraints = node.config['dynamic_constraints']
				# A single fix-up: if the node only has a single
				# string for a constraint, treat it as a one-item
				# list.  In all other cases, constraints are treated
				# as a list.
				if isinstance(constraints, str):
					constraints = [constraints]
				for constraint in constraints:
					if not ConstraintMatches(constraint, hostToMatch):
						return 0

			# If we get here, we have a full match on
			# the unresolved dynamic host name, and a
			# host name for it.  Resolve the host name,
			# and report the match.
			dynamicHosts[hostspec] = hostToMatch
			return 1
	else:
		# The node is specified by name.  Use the specified name.
		actualHost = hostspec

	# This will report whether the two hosts actually match.
	return SameHost(string.lower(actualHost), string.lower(hostToMatch))

def __qualifyHostname__( host ):
	"""__qualifyHostname__(host)
	Converts host to a fully qualified domain name """
	if string.find(host,'.')>=0:
		return host
	else:
		for (prefix, domain) in __hostPrefixPairs__:
			if string.find(host,prefix)==0:
				return "%s%s"%(host,domain)
		return socket.getfqdn(host)


class SPU:
	"""Main class that defines a Stream Processing Unit.

	public functions:

	    Conf:	Sets a key/value list in this SPU's configuration
	    AddServer:  Tells a client node where to find its server.
		AddDisplay: Adds a 'display' to the list of displays (for tilesort)

	"""
	def __init__( self, name ):
		"""Creates a SPU with the given name."""
		self.name = name
		self.config = {}
		self.clientargs = []
		self.servers = []
		self.layoutFunction = None
		self.displays = []

	def Conf( self, key, *values ):
		"""Set a configuration option."""
		# XXX we'll eventually force values to be a single value or a list!
		if type(values) == types.TupleType and len(values) > 1:
			print "***WARNING: Obsolete syntax detected in Conf('%s', ...)!" % key
			print "***WARNING: Put brackets around N-element values (i.e. Python list syntax)."
		if len(values) > 1:
			self.config[key] = list(values)
		else:
			self.config[key] = values[0]

	def __add_server( self, node, url ):
		self.servers.append( (node, url) )

	def AddServer( self, node, protocol='tcpip', port=7000 ):
        	"""AddServer(node, protocol='tcpip', port=7000)
                Tells a client node where to find its server."""
#		if protocol == 'tcpip':
#			self.__add_server( node, "%s://%s:%d" % (protocol,node.ipaddr,port) )
#		elif (protocol.startswith('file') or protocol.startswith('swapfile')):
		if (protocol.startswith('file') or protocol.startswith('swapfile')):
			self.__add_server( node, "%s" % protocol )
			# Don't tell the server "node" about this.
		else:
			# XXX use node.host or node.ipaddr here??? (BP)
			self.__add_server( node, "%s://%s:%d" % (protocol,node.host,port) )
			# use this for tcp/ip : send hostname rather than ip
			# (waiting for getaddrinfo, for probing which one is
			#  available)
		if node != None:
			node.Conf( 'port', port )
			node.AddClient( self, protocol )

	def AddDisplay(self, display_id, w, h, align_matrix, align_matrix_inv):
		"""AddDisplay(display_id, w, h, align_matrix, align_matrix_inv)
		Adds a display with a given id and size to spu, for the 
		tilesort SPU"""
		self.displays.append( (display_id, w, h, align_matrix, align_matrix_inv) )

	def TileLayoutFunction( self, layoutFunc ):
		"""Set the tile layout callback function for a tilesort SPU."""
		# Set the tile layout function for a tilesort SPU
		assert self.name == "tilesort"
		self.layoutFunction = layoutFunc

# This structure will be used just to count how many dynamic host
# indicators we find while creating our SPU graph.  We'll use
# it to know when to signal the main application to continue
# (when all the dynamic hosts have been identified)
dynamicHostsNeeded = { }

class CRNode:
	"""Base class that defines a node in the SPU graph

	public functions:

	    Rank:   Sets the node's rank.
	    AddSPU:	Adds a SPU to the front of the SPU chain.
	    SPUDir:	Sets the directory SPUs start in.
	    AutoStart:	Pass this method a string to start the process
	    		associated with this CRNode from the mothership.
			You can pass a list of strings as the argument
			for use in os.spawnv() or a single string which
			will be split into a list. Make sure the first
			thing you pass is the full path to the executable.

			Examples:
			CRNode dummy( 'jimbobsbox' )
			dummy.AutoStart( "/usr/bin/ssh jimbobsbox crserver" )

			CRNode dummy( 'matilda' )
			dummy.AutoStart( ["/usr/bin/ssh", "matilda", "setenv FILE /Poorly Named/Data.1 ; crserver "] )
	"""
	SPUIndex = 0

	def __init__( self, host ):
	    	"""CRNode(host)
		Creates a node on the given "host"."""
		self.host = host
		if (host == 'localhost'):
			self.host = socket.getfqdn()
		elif host[0:1] == "#":
			# Count how many dynamic nodes have been specified.
			dynamicHostsNeeded[host] = 1

		# unqualify the hostname if it is already that way.
		# e.g., turn "foo.bar.baz" into "foo"
		# Disabled by Brian.
		#period = self.host.find( "." )
		#if period != -1:
		#	self.host = self.host[:period]

		self.SPUs = []
		self.spokenfor = 0
		self.spusloaded = 0
		self.config = {}
		self.tcpip_accept_wait = None
		self.tcpip_connect_wait = None
		self.ib_accept_wait = None
		self.ib_connect_wait = None
		self.gm_accept_wait = None
		self.gm_connect_wait = None
		self.teac_accept_wait = []
		self.teac_connect_wait = []
		self.tcscomm_accept_wait = []
		self.tcscomm_connect_wait = []
		self.alias = host
		self.autostart = "" ;
		self.autostart_argv = [] ;

	def Alias( self, name ):
		self.alias = name
	
	def Rank( self, rank ):
		"""Rank(rank)
		Sets the node's rank."""
		self.config['rank'] = str( rank )

	def AddSPU( self, spu ):
	    	"""AddSPU(spu)
		Adds the given SPU to the front of the SPU chain."""
		self.SPUs.append( spu )
		spu.ID = CRNode.SPUIndex
		spu.node = self

		CRNode.SPUIndex += 1
		allSPUs[spu.ID] = spu

	def Conf( self, key, value ):
		"""Sets a key/value list in this node's configuration"""
		self.config[key] = value

	def SPUDir( self, dir ):
	    	"""SPUDir(dir)
		Sets the directory that SPUs start in."""
		self.Conf('spu_dir', dir)

	def AutoStart( self, program ):
		if type( program ) == types.StringType:
			self.autostart_argv = string.split( program )
			self.autostart = self.autostart_argv[0]
		else:
			self.autostart_argv = program
			self.autostart = program[0]

	def SetPosition(self, x, y):
		# not used by mothership, set by graphical config tool
		pass

class CRNetworkNode(CRNode):
	"""Sub class of CRNode that defines a node in the SPU graph that
	handles incoming and outgoing network traffic.

	public functions:

	    Conf:	Sets a key/value list in this node's configuration
	    AddClient:	Adds a client to the list of clients.
		FileClient: Add a file-readback client
	    AddTile:	Adds a tile to the list of tiles
		AddTileToDisplay: Adds a tile to a specified collection of tiles (a display)

	"""
	def __init__( self, host='localhost' ):
	    	"""CRNetworkNode(host='localhost')
		Creates a network node for the given "host"."""
		CRNode.__init__(self,host)
		self.clients = []
		self.file_clients = []
		self.tiles = []
		self.tiles_on_displays = []

	def AddClient( self, node, protocol ):
		"""AddClient(node, protocol)
		Adds a client node, communicating with "protocol", to the
		list of clients."""
		self.clients.append( (node,protocol) )

	def FileClient( self, fname ):
		"""FileClient(node, fname)
		Adds a file-readback client link from the named file."""
		self.file_clients.append( "file://%s" % fname )

	def AddTile( self, x, y, w, h ):
		"""AddTile(x, y, w, h)
		Defines a tile with the given geometry to be used by a
		tilesort SPU.
		"""
		self.tiles.append( (x,y,w,h) )

	def AddTileToDisplay( self, display_id, x, y, w, h ):
		"""AddTileToDisplay(display_id, x, y, w, h)
		Similar to AddTile, but for use with specifing displays.
		Note that (x, y) are relative to the origin of the 
		display, not the mural!
		"""
		self.tiles_on_displays.append( (display_id,x,y,w,h) )

class CRUTServerNode(CRNode):
	"""Sub class of CRNode that defines a node in the SPU graph that
	handles outgoing network traffic for events.

	public functions:

	    Conf:	Sets a key/value list in this node's configuration
	    AddCRUTClient:	Adds a client to the list of crutclients.
	"""

	def __init__( self, host='localhost' ):
	    	"""CRUTServerNode(host='localhost')
		Creates a network node for the given "host"."""
		CRNode.__init__(self,host)
		self.crutclients = []

	#A crutserver will be creating events, it should be the only server
	def __add_crut_client( self, node, url ):
		self.crutclients.append( (node, url) )

	def AddCRUTClient( self, node, protocol='tcpip', port=9000 ):
		"""AddCRUTClient(node, protocol='tcpip', port=9000)
                Tells a crutserver node where to find a client."""
                self.__add_crut_client( node, "%s://%s:%d" % (protocol,node.host,port) )
		
class CRUTProxyNode(CRNode):
	"""Sub class of CRNode that defines a node in the SPU graph that
	handles incoming and outgoing network traffic for events.

	public functions:

	    Conf:	Sets a key/value list in this node's configuration
	    AddCRUTClient:	Adds a client to the list of clients.
	"""

	def __init__( self, host='localhost' ):
	    	"""CRUTProxyNode(host='localhost')
		Creates a network node for the given "host"."""
		CRNode.__init__(self,host)
		self.crutclients = []
		self.crutservers = []

	def __add_crut_client( self, node, url ):
		self.crutclients.append( (node, url) )

	def AddCRUTClient( self, node, protocol='tcpip', port=9000 ):
		"""AddCRUTClient(node, protocol='tcpip', port=9000)
                Tells a crutproxy node where to find a client."""
                self.__add_crut_client( node, "%s://%s:%d" % (protocol,node.host,port) )

	def __add_crut_server( self, node, url ):
		self.crutservers.append( (node, url) )
		
	def AddCRUTServer( self, node, protocol='tcpip', port=9000 ):
		self.__add_crut_server( node, "%s://%s:%d" % (protocol,node.host,port) )
		if node != None:
			node.AddCRUTClient( self, protocol, port)

class CRApplicationNode(CRNode):
	"""Sub class of CRNode that defines the start of the the SPU graph.

	public functions:

	    SetApplication:	Sets the application that generates the OpenGL.
	    StartDir:		Sets the starting directory of the app.
	    ClientDLL:		Sets the DLL of the client.
	"""
	AppID = 0

	def __init__(self, host='localhost'):
	    	"""CRApplicationNode(host='localhost')
		Creates an application node for the given "host"."""
		CRNode.__init__(self, host)
		self.crutservers = []
		self.crutclients = []
		self.id = CRApplicationNode.AppID
		CRApplicationNode.AppID += 1;
		self.Conf('start_dir', '.')
		self.crut_spokenfor = 0

	def SetApplication( self, app ):
		"""SetApplication(name)
		Sets the name of the application that's run."""
		self.Conf('application', app)

	def StartDir( self, dir ):
		"""SetApplication(dir)
		Sets the directory the application starts in."""
		self.Conf('start_dir', dir)

	def ClientDLL( self, dir ):
		"""Set the directory to search for the crfaker library."""
		self.Conf('client_dll', dir)

	def __add_crut_client( self, node, url ):
		self.crutclients.append( (node, url) )

	def AddCRUTClient( self, node, protocol='tcpip', port=9000 ):
		"""AddCRUTClient(node, protocol='tcpip', port=9000)
                Tells a crutserver node where to find a client."""
                self.__add_crut_client( node, "%s://%s:%d" % (protocol,node.host,port) )

	def __add_crut_server( self, node, url ):
		self.crutservers.append( (node, url) )
		
	def AddCRUTServer( self, node, protocol='tcpip', port=9000 ):
		self.__add_crut_server( node, "%s://%s:%d" % (protocol,node.host,port) )
		if node != None:
			node.AddCRUTClient( self, protocol, port)

class SockWrapper:
	"Internal convenience class for handling sockets"
	NOERROR_MORE = 100
	NOERROR = 200
	UNKNOWNHOST = 400
	NOTHINGTOSAY = 401
	UNKNOWNCOMMAND = 402
	UNKNOWNSPU = 403
	UNKNOWNPARAM = 404
	UNKNOWNSERVER = 405
	UNKNOWNPROTOCOL = 406
	NOAPPLICATION = 407
	INVALIDPARAM = 408

	def __init__(self, sock):
		self.sock = sock
		self.file = sock.makefile( "r" )
		self.SPUid = -1
		self.node = None
		self.tcpip_accept_wait = None
		self.tcpip_connect_wait = None
		self.ib_accept_wait = None
		self.ib_connect_wait = None
		self.gm_accept_wait = None
		self.gm_connect_wait = None
		self.teac_accept_wait = []
		self.teac_connect_wait = []
		self.tcscomm_accept_wait = []
		self.tcscomm_connect_wait = []

	def readline( self ):
		return string.strip(self.file.readline())

	def Send(self, str):
		self.sock.send( str + "\n" )

	def Reply(self, code, s=None):
		tosend = `code`
		if s != None:
			tosend += " " + str(s)
		self.Send( tosend )
		CRDebug( 'Replying (%d): "%s"' % ( code, s ) )

	def Success( self, msg ):
		self.Reply( SockWrapper.NOERROR, msg )

	def MoreComing( self, msg ):
		self.Reply( SockWrapper.NOERROR_MORE, msg )

	def Failure( self, code, msg ):
		self.Reply( code, msg )

class CRSpawner(threading.Thread):
	"""A class used to start processes on nodes.

	Since the mothership knows what should be running on each node, it
	can start these processes if you tell it how to start a job remotely.
	Each CRNode now has members named autostart and autostart_argv. If
	you set these to non-null strings, the spawner will call os.spawnv()
	with these values when the spawner's run() member is called.

	The autostart member should be a string containing the full path
	to an executable program to be run, i.e. "/usr/bin/ssh". The
	autostart_argv member should be a vector containing the argument
	list for the program, i.e.,
		( "ssh", "mynode.mydotcom.com", "/usr/local/bin/crserver" )
	NOTE: Yes, the program name should be the zeroth item in the list
	of arguments, which means it is repeated.

	Since a new process is created, there is no need for your program
	to complete before the application finishes.
	"""
	def __init__( self, nodes, branches=0, maxnodes=1):
		self.maxnodes = maxnodes
		self.branches = branches
		self.nodes = []
		self.count = 0
		for node in nodes:
			self.nodes.append( node )
			self.count = self.count + 1
		threading.Thread.__init__(self)
	def run( self ):
		if self.branches < 2 or self.count <= self.maxnodes:
			# This thread will sequentially spawn all listed nodes.
			for node in self.nodes:
				if node.autostart != "":
					os.spawnv( os.P_NOWAIT, node.autostart, node.autostart_argv )
					CRInfo("Autostart for node %s: %s" % (node.host, str(node.autostart_argv)))
				else:
					if isinstance(node, CRNetworkNode):
						CRInfo("Start a crserver on %s" % node.host)
					elif isinstance(node, CRUTServerNode):
						CRInfo("Start a crutserver on %s" % node.host)
					elif isinstance(node, CRUTProxyNode):
						CRInfo("Start a crutproxy on %s" % node.host)
					else:
						CRInfo("Start a crappfaker on %s" % node.host)
		else:
			# We have more nodes than we want to handle in this
			# thread.  Instead of spawning processes, create new
			# threads, and have those threads handle pieces of the
			# nodes.
			childsize = int((self.count + self.branches - 1)/self.branches)
			for i in range(0, self.count, childsize):
				child = CRSpawner(self.nodes[i:i+childsize], self.branches, self.maxnodes)
				child.start()

class CR:
	"""Main class that controls the mothership

        Most of the mothership network communication takes the form of
        incoming strings that the mothership responds to with answer
        strings.  The do_* functions handle this communication language.

	public functions:
	    AddNode: Adds a node to the SPU graph.
	    MTU: Sets the maximum communication buffer size.
	    Go: Starts the ball rolling.
	    AllSPUConf: Adds the key/values list to all SPUs' configuration.
	    Conf: Set a mothership parameter
	    GetConf: Return value of a mothership parameter
	    ContextRange: Sets the Quadrics context range.
	    NodeRange: Sets the Quadrics node range.
	    CommKey: Sets the Quadrics communication key

	internal functions:
            ProcessRequest:     Handles an incoming request, mapping it to
                                an appropriate do_* function.
	    do_acceptrequest:	Accepts the given socket.
	    do_clients: 	Sends the list of clients to a server.
	    do_connectrequest:	Connects the given socket.
	    do_faker:		Maps a faker app to an ApplicationNode.
	    do_opengldll:	Identifies the application node in the graph.
	    do_rank:            Sends the node's rank down.
	    do_quit: 		Disconnects from clients.
	    do_reset: 		Resets the mothership to its initial state.
	    do_server:		Identifies the server in the graph.
	    do_newserver:	Identifies a new server for replication.
	    do_serverids:	Sends the list of server IDs.
	    do_serverparam:	Sends the given server parameter.
	    do_fakerparam:	Sends the given app faker parameter.
	    do_servers: 	Sends the list of servers.
	    do_servertiles: 	Sends the defined tiles for a server.
	    do_spu:		Identifies a SPU.
	    do_spuparam:	Sends the given SPU (or global) parameter.
	    do_tiles:		Sends the defined tiles for a SPU.
	    do_setparam:        Sets a mothership parameter value
	    do_getparam:        Returns a mothership parameter value
	    do_logperf:		Logs Performance Data to a logfile.
	    do_gettilelayout:   Calls the user's LayoutTiles() function and returns
		                   the list of new tiles.
		 do_getstatus:		Returns information about the state of the nodes.
	    tileReply: 		Packages up a tile message for socket communication.
	    ClientDisconnect: 	Disconnects from a client
	"""

	startupCallbacks = []

	def __init__( self ):
		self.nodes = []
		self.all_sockets = []
		self.wrappers = {}
		self.allSPUConf = []
		self.daughters = []
		self.conn_id = 1
		self.enable_autostart = 1
		self.config = {"MTU" : 1024 * 1024,
			       "low_context" : 32,
			       "high_context" : 35,
			       "low_node" : "iam0",
			       "high_node" : "iamvis20",
			       "comm_key": [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
				   "autostart_branches": 0,
				   "autostart_max_nodes_per_thread": 1}
		# This is set only on daughterships
		self.mother = None

	def AddNode( self, node ):
		"""AddNode(node)
		Adds a node to the SPU graph."""
		self.nodes.append( node )
	
	def Conf( self, key, value ):
		"""Set a global mothership configuration value (via Python)"""
		appOptions = ["minimum_window_size",
					  "match_window_title",
					  "track_window_size",
					  "show_cursor"]
		try:
			i = appOptions.index(key)
		except:
			# this is a mothership option
			self.config[key] = value
		else:
			# this is an app node option (backward compatibility hack)
			print "NOTICE: %s is an obsolete mothership option; it's now an app node option" % key
			for node in self.nodes:
				if isinstance(node, CRApplicationNode):
					node.Conf(key, value)

	def ContextRange( self, low_context, high_context ):
		"""ContextRange( low_context, high_context )
		Sets the context range to use with Elan."""
		self.config["low_context"]  = low_context;
		self.config["high_context"] = high_context;

	def NodeRange( self, low_node, high_node ):
		"""NodeRange( low_node, high_node )
		Sets the node range to use with Elan."""
		period = low_node.find( "." )
		if period != -1:
			low_node = low_node[:period]
		self.config["low_node"]  = low_node;
		period = high_node.find( "." )
		if period != -1:
			high_node = high_node[:period]
		self.config["high_node"] = high_node;

	def CommKey( self, byteList ):
		"""CommKey( [byte0, byte1, ..., byte15] )
		Sets the user key to use with Elan."""
		self.config["comm_key"]= byteList
		CRDebug("Setting comm key to %s"%str(byteList))

	def AllSPUConf( self, regex, key, *values ):
		"""AllSPUConf(regex, key, *values)
		Adds the key/values list to the global SPU configuration."""
		self.allSPUConf.append( (regex, key, map( MakeString, values) ) )

	# XXX obsolete; use Conf('MTU', value) instead
	def MTU( self, mtu ):
		"""MTU(size)
		Sets the maximum buffer size allowed in communication
		between SPUs."""
		self.Conf("MTU", mtu)

	# XXX obsolete; use Conf() instead
	def SetParam( self, key, value ):
		print "NOTICE: cr.SetParam() is obsolete; use cr.Conf() instead."
		self.Conf(key, value)

	# Added by BrianP
	def do_setparam( self, sock, args ):
		"""Set a global mothership parameter value (via C)"""
		params = args.split( " ", 1 )
		key = params[0]
		value = params[1]
		self.Conf(key, value)
		sock.Success( "OK" )
		return
		
	# Added by BrianP
	def do_getparam( self, sock, args ):
		"""Get a global mothership parameter value (via C)"""
		key = args
		if not self.config.has_key(key):
			response = ""
		else:
			response = str(self.config[key])
		sock.Success( response )
		return

	def Go( self, PORT = -1 ):
		"""Go(PORT=10000)
		Starts the ball rolling.
		This starts the mothership's event loop."""
		if self.mother:
			CRInfo("This is Chromium Daughtership, Version 1.5")
		else:
			CRInfo("This is Chromium, Version 1.5")
		try:
			if PORT == -1:
				# Port was not specified.  Get it from
				# CRMOTHERSHIP environment variable if possible..
				if os.environ.has_key('CRMOTHERSHIP'):
					motherString = os.environ['CRMOTHERSHIP']
					loc = string.find(motherString,':')
					if loc >= 0:
						try:
							PORT = int(motherString[loc+1:])
							CRDebug("Using PORT %d"%PORT)
						except Exception, val:
							CRInfo("Could not parse port number from <%s>: %s"%(motherString,val))
							CRInfo("Using default PORT!")
							PORT = 10000
					else:
						PORT = 10000 # default value
				else:
					PORT = 10000  # default value

			for res in socket.getaddrinfo(None, PORT, socket.AF_UNSPEC, socket.SOCK_STREAM, 0, socket.AI_PASSIVE):
				(af, socktype, proto, canonname, sa) = res

				try:
					s = socket.socket( af, socktype )
				except:
					CRDebug( "Couldn't create socket of family %u, trying another one" % af );
					continue

				try:
					s.setsockopt( socket.SOL_SOCKET, socket.SO_REUSEADDR, 1 )
				except:
					CRDebug( "Couldn't set the SO_REUSEADDR option on the socket!" )
					continue

				try:
					s.bind( sa )
				except:
					CRDebug( "Couldn't bind to port %d" % PORT );
					continue

				try:
					s.listen(100)
				except:
					CRDebug( "Couldn't listen!" );
					continue

				#CRDebug( "Mothership ready" );
				self.all_sockets.append(s)

				# Call any callbacks which may have been
				# set via CRAddStartupCallback()
				for cb in CR.startupCallbacks:
					cb(self)

				# Create a single thread that will then go
				# spawn nodes (for autostart nodes, this will
				# actually start the servers or applications
				# itself; for manual start nodes, a message
				# will be printed directing the user to start
				# the appropriate executable).
				#
				# This thread will either sequentially handle
				# all nodes (by default, if autostart_branches=None)
				# or will create a number of new threads (quite
				# possibly recursively) to handle subsets of the
				# nodes (if autostart_branches is greater than 1, 
				# this will be a tree of threads).
				if self.enable_autostart:
					spawner = CRSpawner(self.nodes, self.config['autostart_branches'], self.config['autostart_max_nodes_per_thread'])
					spawner.start()

				# If we're supposed to "phone home" with a signal, do so
				# with the USR1 signal.  This will happen when we're
				# auto-starting on Linux - the OpenGL stub will wait
				# until the mothership is going before attempting to
				# make contact.  (The CRSIGNAL envariable should never
				# be set on Windows, since Windows Python doesn't
				# seem to support os.kill().)
				needToSignal = 0
				if os.environ.has_key('CRSIGNAL'):
					needToSignal = 1

				while 1:
					# We can only safely signal the mothership when all the
					# dynamic nodes have been resolved; this is because the
					# main application will ask about what servers are available,
					# and we don't know the answer until all dynamic nodes are
					# resolved.  Note that this essentially prevents Windows
					# users from using dynamic hosts, because they cannot signal.
					if needToSignal and len(dynamicHosts) == len(dynamicHostsNeeded):
						process = int(os.environ['CRSIGNAL'])
						CRInfo("Mothership signalling spawning process %d" % process)
						os.kill(process,signal.SIGUSR1)
						needToSignal = 0

					ready = select.select( self.all_sockets, [], [], 0.1 )[0]
					for sock in ready:
						if sock == s:
							# accept a new connection
							conn, addr = s.accept()
							self.wrappers[conn] = SockWrapper(conn)
							self.all_sockets.append( conn )
						else:
							# process request from established connection
							self.ProcessRequest( self.wrappers[sock] )

			Fatal( "Couldn't find local TCP port (make sure that another mothership isn't already running)")
		except KeyboardInterrupt:
			try:
				for sock in self.all_sockets:
					sock.shutdown(2)
					sock.close( )
			except:
				pass
			CRInfo("\n\nThank you for using Chromium!")
		except:
			CRInfo("\n\nMOTHERSHIP EXCEPTION!  TERRIBLE!")
			traceback.print_exc(None, sys.stderr)
			try:
				for sock in self.all_sockets:
					sock.shutdown(2)
					sock.close( )
			except:
				pass

	def ClientError( self, sock_wrapper, code, msg ):
		"""ClientError(sock_wrapper, code, msg)
		Sends an error message on the given socket."""
		sock_wrapper.Reply( code, msg )
		self.ClientDisconnect( sock_wrapper )
		
	def ClientDisconnect( self, sock_wrapper ):
		"""ClientDisconnect(sock_wrapper)
		Disconnects from the client on the given socket."""
		self.all_sockets.remove( sock_wrapper.sock )
		del self.wrappers[sock_wrapper.sock]
		try:
			sock_wrapper.sock.close( )
		except:
			pass

	def do_connectrequest( self, sock, args ):
		"""do_connectrequest(sock, args)
		Connects the given socket."""
		connect_info = args.split( " " )
		protocol = connect_info[0]
		if (protocol == 'tcpip' or protocol == 'udptcpip'):
			(p, hostname, port_str, endianness_str) = connect_info
			hostname = socket.gethostbyname(__qualifyHostname__(hostname))
			port = int(port_str)
			endianness = int(endianness_str)
			for server_sock in self.wrappers.values():
				if server_sock.tcpip_accept_wait != None:
					(server_hostname, server_port, server_endianness) = server_sock.tcpip_accept_wait
					if SameHost(server_hostname, hostname) and server_port == port:
						sock.Success( "%d %d" % (self.conn_id, server_endianness ) )
						server_sock.Success( "%d" % self.conn_id )
						self.conn_id += 1
						return
					else:
						CRDebug( "not connecting to \"%s:%d\" (!= \"%s:%d\")" % (server_hostname, server_port, hostname, port) )
			sock.tcpip_connect_wait = (hostname, port, endianness)
		elif (protocol == 'ib'):
			(p, hostname, port_str, endianness_str) = connect_info
			CRInfo("do_connectrequest processing ib protocol")
			hostname = socket.gethostbyname(hostname)
			port = int(port_str)
			endianness = int(endianness_str)
			for server_sock in self.wrappers.values():
				if server_sock.ib_accept_wait != None:
					(server_hostname, server_port, server_endianness) = server_sock.ib_accept_wait
					if SameHost(server_hostname, hostname) and server_port == port:
						sock.Success( "%d %d" % (self.conn_id, server_endianness ) )
						server_sock.Success( "%d" % self.conn_id )
						self.conn_id += 1
						return
					else:
						CRDebug( "not connecting to \"%s:%d\" (!= \"%s:%d\")" % (server_hostname, server_port, hostname, port) )
			sock.ib_connect_wait = (hostname, port, endianness)
		elif (protocol == 'gm'):
			(p, hostname, port_str, node_id_str, port_num_str, endianness_str) = connect_info
			port = int(port_str)
			node_id = int(node_id_str)
			port_num = int(port_num_str)
			endianness = int(endianness_str)
			for server_sock in self.wrappers.values():
				if server_sock.gm_accept_wait != None:
					(server_hostname, server_port, server_node_id, server_port_num, server_endianness) = server_sock.gm_accept_wait
					if SameHost(server_hostname, hostname) and server_port == port:
						sock.Success( "%d %d %d %d" % (self.conn_id, server_node_id, server_port_num, server_endianness) )
						server_sock.Success( "%d %d %d" % (self.conn_id, node_id, port_num) )
						server_sock.gm_accept_wait = None
						self.conn_id += 1
						return
			sock.gm_connect_wait = (hostname, port, node_id, port_num, endianness)
		elif (protocol == 'quadrics'):
			(p, remote_hostname, remote_rank_str, my_hostname, my_rank_str, my_endianness_str) = connect_info
			remote_rank = int(remote_rank_str)
			my_rank = int(my_rank_str)
			my_endianness = int(my_endianness_str)
			for server_sock in self.wrappers.values():
				if server_sock.teac_accept_wait != []:
					(server_hostname, server_rank, server_endianness) = server_sock.teac_accept_wait[0]
					if SameHost(server_hostname, remote_hostname) and server_rank == remote_rank:
						server_sock.teac_accept_wait.pop(0)
						sock.Success( "%d %d" % (self.conn_id, server_endianness) )
						server_sock.Success( "%d %s %d %d" % (self.conn_id, my_hostname, my_rank, my_endianness) )
						self.conn_id += 1
						return
			sock.teac_connect_wait.append( (my_hostname, my_rank, my_endianness, remote_hostname, remote_rank) )
		elif (protocol == 'quadrics-tcscomm'):
			(p, remote_hostname, remote_rank_str, my_hostname, my_rank_str, my_endianness_str) = connect_info
			remote_rank = int(remote_rank_str)
			my_rank = int(my_rank_str)
			my_endianness = int(my_endianness_str)
			for server_sock in self.wrappers.values():
				if server_sock.tcscomm_accept_wait != []:
					(server_hostname, server_rank, server_endianness) = server_sock.tcscomm_accept_wait[0];
					if SameHost(server_hostname, remote_hostname) and server_rank == remote_rank:
						server_sock.tcscomm_accept_wait.pop(0);
						sock.Success( "%d %d" % (self.conn_id, server_endianness) )
						server_sock.Success( "%d %s %d %d" % (self.conn_id, my_hostname, my_rank, my_endianness) )
						self.conn_id += 1
						return
			sock.tcscomm_connect_wait.append( (my_hostname, my_rank, my_endianness, remote_hostname, remote_rank) )
		else:
			sock.Failure( SockWrapper.UNKNOWNPROTOCOL, "Never heard of protocol %s" % protocol )

	def do_acceptrequest( self, sock, args ):
		"""do_acceptrequest(sock, args)
		Accepts the given socket."""
		accept_info = args.split( " " )
		protocol = accept_info[0]
		if protocol == 'tcpip' or protocol == 'udptcpip':
			(p, hostname, port_str, endianness_str) = accept_info
			hostname = socket.gethostbyname(__qualifyHostname__(hostname))
			port = int(port_str)
			endianness = int(endianness_str)
			for client_sock in self.wrappers.values():
				if client_sock.tcpip_connect_wait != None:
					(client_hostname, client_port, client_endianness) = client_sock.tcpip_connect_wait
					if SameHost(client_hostname, hostname) and client_port == port:
						sock.Success( "%d" % self.conn_id )
						client_sock.Success( "%d %d" % (self.conn_id, endianness )  )
						self.conn_id += 1
						return
					else:
						CRDebug( "not accepting from \"%s:%d\" (!= \"%s:%d\")" % (client_hostname, client_port, hostname, port ) )
				else:
					CRDebug( "tcpip_connect_wait" )
						
			sock.tcpip_accept_wait = (hostname, port, endianness)
		elif protocol == 'ib':
			(p, hostname, port_str, endianness_str) = accept_info
			CRInfo("do_acceptrequest processing ib protocol")
			hostname = socket.gethostbyname(hostname)
			port = int(port_str)
			endianness = int(endianness_str)
			for client_sock in self.wrappers.values():
				if client_sock.ib_connect_wait != None:
					(client_hostname, client_port, client_endianness) = client_sock.ib_connect_wait
					if SameHost(client_hostname, hostname) and client_port == port:
						sock.Success( "%d" % self.conn_id )
						client_sock.Success( "%d %d" % (self.conn_id, endianness )  )
						self.conn_id += 1
						return
					else:
						CRDebug( "not accepting from \"%s:%d\" (!= \"%s:%d\")" % (client_hostname, client_port, hostname, port ) )
				else:
					CRDebug( "ib_connect_wait" )
						
			sock.ib_accept_wait = (hostname, port, endianness)
		elif protocol == 'gm':
			(p, hostname, port_str, node_id_str, port_num_str, endianness_str) = accept_info
			port = int(port_str)
			node_id = int(node_id_str)
			port_num = int(port_num_str)
			endianness = int(endianness_str)
			for client_sock in self.wrappers.values():
				if client_sock.gm_connect_wait != None:
					(client_hostname, client_port, client_node_id, client_port_num, client_endianness) = client_sock.gm_connect_wait
					if SameHost(client_hostname, hostname) and client_port == port:
						sock.Success( "%d %d %d" % (self.conn_id, client_node_id, client_port_num) )
						client_sock.Success( "%d %d %d %d" % (self.conn_id, node_id, port_num, endianness) )
						self.conn_id += 1
						client_sock.gm_connect_wait = None
						return
			sock.gm_accept_wait = (hostname, port, node_id, port_num, endianness)
		elif protocol == 'quadrics':
			(p, hostname, rank_str, endianness_str) = accept_info
			rank = int(rank_str)
			endianness = int(endianness_str)
			for client_sock in self.wrappers.values():
				if client_sock.teac_connect_wait != []:
					(client_hostname, client_rank, client_endianness, server_hostname, server_rank) = client_sock.teac_connect_wait[0];
					if SameHost(server_hostname, hostname) and server_rank == rank:
						client_sock.teac_connect_wait.pop(0)
						sock.Success( "%d %s %d %d" % (self.conn_id, client_hostname, client_rank, client_endianness) )
						client_sock.Success( "%d %d" % (self.conn_id, endianness) )
						self.conn_id += 1
						return
			sock.teac_accept_wait.append( (hostname, rank, endianness) )
		elif protocol == 'quadrics-tcscomm':
			(p, hostname, rank_str, endianness_str) = accept_info
			rank = int(rank_str)
			endianness = int(endianness_str)
			for client_sock in self.wrappers.values():
				if client_sock.tcscomm_connect_wait != []:
					(client_hostname, client_rank, client_endianness, server_hostname, server_rank) = client_sock.tcscomm_connect_wait[0]
					if SameHost(remote_hostname, hostname) and remote_rank == rank:
						client_sock.tcscomm_connect_wait.pop(0)
						sock.Success( "%d %s %d %d" % (self.conn_id, client_hostname, client_rank, client_endianness) )
						client_sock.Success( "%d %d" % (self.conn_id, my_endianness) )
						self.conn_id += 1
						return
			sock.tcscomm_accept_wait.append( (hostname, rank, endianness) )
		else:
			sock.Failure( SockWrapper.UNKNOWNPROTOCOL, "Never heard of protocol %s" % protocol )

	def do_faker( self, sock, args ):
		"""do_faker(sock, args)
		Maps the incoming "faker" app to a previously-defined node."""
		for node in self.nodes:
			if not node.spokenfor and isinstance(node,CRApplicationNode) and MatchNode(node,args):
				try:
					application = node.config['application']
				except:
					sock.Failure( SockWrapper.NOAPPLICATION, "Client node has no application!" )
					return
				node.spokenfor = 1
				sock.node = node
				sock.Success( "%d %s" % (node.id, application) )
				return
		sock.Failure( SockWrapper.UNKNOWNHOST, "Never heard of faker host %s" % args )

	def do_newserver( self, sock, args ):
		"""do_newserver(sock, args)
		Identifies a new server for replication. """
		sock.Success( "1 1 render" )

	def do_crutproxy( self, sock, args ):
		CRDebug ( " Seeing if we have a crutproxy." )
		"""do_crutserver(sock, args)
		Hopefully tells us that we have a crutserver running somewhere."""
		for node in self.nodes:
			if not node.spokenfor and isinstance(node,CRUTProxyNode) and MatchNode(node,args):
				node.spokenfor = 1
				sock.node = node
				sock.Success( " " )
				return
		sock.Failure( SockWrapper.UNKNOWNHOST, "Never heard of crutproxy host %s" % args )
		
	def do_crutserver( self, sock, args ):
		"""do_crutserver(sock, args)
		Hopefully tells us that we have a crutserver running somewhere."""
		for node in self.nodes:
			if not node.spokenfor and isinstance(node,CRUTServerNode) and MatchNode(node,args):
				node.spokenfor = 1
				sock.node = node
				sock.Success( " " )
				return
		sock.Failure( SockWrapper.UNKNOWNHOST, "Never heard of crutserver host %s" % args )

	def do_crutclient( self, sock, args ):
		"""do_crutserver(sock, args)
		Hopefully tells us that we have a crutclient running somewhere."""
		for node in self.nodes:
			if not node.crut_spokenfor and isinstance(node,CRApplicationNode) and len(node.crutservers) > 0 and MatchNode(node,args):
				node.crut_spokenfor = 1
				sock.node = node
				sock.Success( " " )
				return
		sock.Failure( SockWrapper.UNKNOWNHOST, "Never heard of crutclient host %s" % args )

	def do_server( self, sock, args ):
		"""do_server(sock, args)
		Identifies the server in the graph. """
		nodenames = ""
		for node in self.nodes:
			nodenames += node.host+" "
			if not node.spokenfor and isinstance(node,CRNetworkNode) and MatchNode(node,args):
				node.spokenfor = 1
				node.spusloaded = 1
				sock.node = node

				spuchain = "%d" % len(node.SPUs)
				for spu in node.SPUs:
					spuchain += " %d %s" % (spu.ID, spu.name)
				sock.Success( spuchain )
				return
                # Wasn't able to find the server.  Figure out what ones
                # were expected.
		sock.Failure( SockWrapper.UNKNOWNHOST, "Never heard of server host %s.  Expected one of: %s" % (args, nodenames))

	def do_opengldll( self, sock, args ):
		"""do_opengldll(sock, args)
		XXX Is this documentation right??!  Not sure. (ahern)

		Identifies the application node in the graph.
		Also, return the client's SPU chain."""
		(id_string, hostname) = args.split( " " )
		app_id = int(id_string)
		for node in self.nodes:
			if isinstance(node,CRApplicationNode):
				if ((app_id == -1 and SameHost(hostname, node.host)) or node.id == app_id) and not node.spusloaded:
					node.spusloaded = 1
					spuchain = "%d" % len(node.SPUs)
					for spu in node.SPUs:
						spuchain += " %d %s" % (spu.ID, spu.name)
					sock.Success( spuchain )
					sock.node = node
					return
		sock.Failure( SockWrapper.UNKNOWNHOST, "Never heard of OpenGL DLL for application %d" % app_id )

	def do_spu( self, sock, args ):
		"""do_spu(sock, args)
		Identifies a SPU."""
		try:
			spuid = int(args)
		except:
			sock.Failure( SockWrapper.UNKNOWNSPU, "Bogus SPU name: %s" % args )
			return
		if not allSPUs.has_key( spuid ):
			sock.Failure( SockWrapper.UNKNOWNSPU, "Never heard of SPU %d" % spuid )
			return
		sock.SPUid = spuid
		sock.Success( "Hello, %s SPU!" % allSPUs[spuid].name )

	def do_spuparam( self, sock, args ):
		"""do_spuparam(sock, args)
		Sends the given SPU (or global) parameter."""
		if sock.SPUid == -1:
			sock.Failure( SockWrapper.UNKNOWNSPU, "You can't ask for SPU parameters without telling me what SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		if not spu.config.has_key( args ):
                        # Okay, there's no specific parameter for the SPU.
                        # Try the global SPU configurations.
			for (regex, key, values) in self.allSPUConf:
				if args == key and re.search( regex, spu.name ) != -1:
					response = values
					break
			else:
				sock.Failure( SockWrapper.UNKNOWNPARAM,
                                            "SPU %d (%s) doesn't have param %s"
                                            % (sock.SPUid,
                                               allSPUs[sock.SPUid].name,
                                               args) )
                                return
		else:
			response = spu.config[args]
		CRDebug("responding with args = " + `response`)
#		sock.Success( string.join( response, " " ) )
		sock.Success( response )

	def do_crutserverparam( self, sock, args ):
		"""do_crutserverparam(sock, args)
		Sends the given crutserver parameter."""
		if sock.node == None or not isinstance(sock.node,CRUTServerNode):
			sock.Failure( SockWrapper.UNKNOWNSERVER, "You can't ask for server parameters without telling me what crutserver you are!" )
			return
		if not sock.node.config.has_key( args ):
			sock.Failure( SockWrapper.UNKNOWNPARAM, "Server doesn't have param %s" % (args) )
			return
		#sock.Success( string.join( sock.node.config[args], " " ) )
		sock.Success( sock.node.config[args] )

	def do_serverparam( self, sock, args ):
		"""do_serverparam(sock, args)
		Sends the given server parameter."""
		if sock.node == None or not isinstance(sock.node,CRNetworkNode):
			sock.Failure( SockWrapper.UNKNOWNSERVER, "You can't ask for server parameters without telling me what server you are!" )
			return
		if not sock.node.config.has_key( args ):
			sock.Failure( SockWrapper.UNKNOWNPARAM, "Server doesn't have param %s" % (args) )
			return
		#sock.Success( string.join( sock.node.config[args], " " ) )
		sock.Success( sock.node.config[args] )

	def do_fakerparam( self, sock, args ):
		"""do_fakerparam(sock, args)
		Sends the given app faker parameter."""
		if sock.node == None or not isinstance(sock.node,CRApplicationNode):
			sock.Failure( SockWrapper.UNKNOWNSERVER, "You can't ask for faker parameters without telling me what app faker you are!" )
			return
		if not sock.node.config.has_key( args ):
			sock.Failure( SockWrapper.UNKNOWNPARAM, "Faker doesn't have param %s" % (args) )
			return
		sock.Success( sock.node.config[args] )

	def do_servers( self, sock, args ):
		"""do_servers(sock, args)
		Sends the list of servers."""
		if sock.SPUid == -1:
			sock.Failure( SockWrapper.UNKNOWNSPU, "You can't ask for servers without telling me what SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		if len(spu.servers) == 0:
			sock.Failure( SockWrapper.UNKNOWNPARAM, "SPU %d doesn't have servers!" % (sock.SPUid) )
			return

		servers = "%d " % len(spu.servers)
		for i in range(len(spu.servers)):
			(node, url) = spu.servers[i]
			# The URL may include a dynamic host reference.  Replace it if it does.
			match = re.search("://(#[^:]*)", url)
			if match:
				# Replace the dynamic spec with its real counterpart
				url = string.replace(url, match.group(1), dynamicHosts[match.group(1)])
			servers += "%s" % (url)
			if i != len(spu.servers) -1:
				servers += ','
		sock.Success( servers )

	def do_crutservers( self, sock, args ):
       		if len(sock.node.crutservers) == 0:
			sock.Failure( SockWrapper.UNKNOWNPARAM, "CRUTClient %d doesn't have servers" % (sock.SPUid) )
			return

		crutservers = "%d " % len(sock.node.crutservers)
		for i in range(len(sock.node.crutservers)):
			(node,url) = sock.node.crutservers[i]
			crutservers+= "%s" % (url)
			if i != len(sock.node.crutservers) -1:
				crutservers += " "
		sock.Success( crutservers )

	def do_crutclients(self, sock, args ):
		#don't error here, you may not have any clients (e.g. last node in fan configuration)
		if len(sock.node.crutclients) == 0:
			sock.Success("0 CRUTserver doesn't have clients.")
			return

		crutclients = "%d " % len(sock.node.crutclients)
		for i in range(len(sock.node.crutclients)):
			(nocde,url) = sock.node.crutclients[i]
			crutclients += "%s" % (url)
			if i != len(sock.node.crutclients) -1:
				crutclients += " "
		sock.Success( crutclients )

	def do_serverids( self, sock, args ):
		"""do_serverids(sock, args)
		Sends the list of server IDs.
		XXX How is this different from do_servers? (ahern)
		"""
		# XXX this might only be temporary (BrianP)
		if sock.SPUid == -1:
			sock.Failure( SockWrapper.UNKNOWNSPU, "You can't ask for server ids without telling me what SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		if len(spu.servers) == 0:
			sock.Failure( SockWrapper.UNKNOWNPARAM, "SPU %d doesn't have servers!" % (sock.SPUid) )
			return

		servers = "%d " % len(spu.servers)
		for i in range(len(spu.servers)):
			(node, url) = spu.servers[i]
			if node == None:
				sock.Failure( SockWrapper.UNKNOWNSERVER, "Sorry, I don't know what SPU the server is running, you didn't tell me." )
				return
			servers += "%d" % (node.SPUs[0].ID)
			if i != len(spu.servers) - 1:
				servers += ' '
		sock.Success( servers )

	def do_tiles( self, sock, args ):
		"""do_tiles(sock, args)
		Sends the defined tiles for a SPU."""
		if sock.SPUid == -1:
			sock.Failure( SockWrapper.UNKNOWNSPU, "You can't ask for tiles without telling me what SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		if len(spu.servers) == 0:
			sock.Failure( SockWrapper.UNKNOWNPARAM, "SPU %d doesn't have servers!" % (sock.SPUid) )
			return
		server_num = int(args)
		if server_num < 0 or server_num >= len(spu.servers):
			sock.Failure( SockWrapper.UNKNOWNSERVER, "SPU %d doesn't have a server numbered %d" % (sock.SPUid, server_num) )
		(node, url) = spu.servers[server_num]
		self.tileReply( sock, node )

	def do_servertiles( self, sock, args ):
		"""do_servertiles(sock, args)
		Sends the defined tiles for a server."""
		if sock.node == None or not isinstance(sock.node,CRNetworkNode):
			sock.Failure( SockWrapper.UNKNOWNSERVER, "You can't ask for tiles without telling me what server you are!" )
			return
		self.tileReply( sock, sock.node )

	def tileReply( self, sock, node ):
		"""tileReply(sock, node)
		Packages up a tile message for socket communication.
		"""
		if len(node.tiles) == 0:
			sock.Failure( SockWrapper.UNKNOWNPARAM, "server doesn't have tiles!" )
			return
		tiles = "%d " % len(node.tiles)
		for i in range(len(node.tiles)):
			tile = node.tiles[i]
			tiles += "%d %d %d %d" % tile
			if i != len(node.tiles) - 1:
				tiles += ","
		sock.Success( tiles )

	def do_serverdisplaytiles( self, sock, args ):
		"""do_servertiles(sock, args)
		Sends the defined tiles for a server."""
		if sock.node == None or not isinstance(sock.node,CRNetworkNode):
			sock.Failure( SockWrapper.UNKNOWNSERVER, "You can't ask for tiles without telling me what server you are!" )
			return
		self.displaytileReply( sock, sock.node )

	def displaytileReply( self, sock, node ):
		"""tileReply(sock, node)
		Packages up a tile message for socket communication.
		"""
		if len(node.tiles_on_displays) == 0:
			sock.Failure( SockWrapper.UNKNOWNPARAM, "server doesn't have tiles!" )
			return
		tiles = "%d " % len(node.tiles_on_displays)
		for i in range(len(node.tiles_on_displays)):
			tile = node.tiles_on_displays[i]
			tiles += "%d %d %d %d %d" % tile
			if i != len(node.tiles) - 1:
				tiles += ","
		sock.Success( tiles )

	def do_displays( self, sock, args ):
		"""do_displays(sock, args)
		Send the displays associated with a SPU"""
		n_displays = 0;
		for spu in range(len(allSPUs)):
			n_displays += len(allSPUs[spu].displays)
		displays = "%d " % n_displays

		for spu in range(len(allSPUs)):
			for i in range(len(allSPUs[spu].displays)):
				display = allSPUs[spu].displays[i]
				
				tmp_display = "%d %d %d %s %s" % display

				reggie = re.compile('\]|\[|,')
				displays += "%s" % reggie.sub(' ', tmp_display)
				
				if i != len(allSPUs[spu].displays) - 1:
					displays += ","
		sock.Success( displays )
		
	def do_display_tiles( self, sock, args ):
		"""do_tiles(sock, args)
		Sends the defined tiles for a SPU."""
		if sock.SPUid == -1:
			sock.Failure( SockWrapper.UNKNOWNSPU, "You can't ask for tiles without telling me what SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		if len(spu.servers) == 0:
			sock.Failure( SockWrapper.UNKNOWNPARAM, "SPU %d doesn't have servers!" % (sock.SPUid) )
			return
		server_num = int(args)
		if server_num < 0 or server_num >= len(spu.servers):
			sock.Failure( SockWrapper.UNKNOWNSERVER, "SPU %d doesn't have a server numbered %d" % (sock.SPUid, server_num) )
		(node, url) = spu.servers[server_num]
		self.displayTileReply( sock, node )

	def displayTileReply( self, sock, node ):
		"""displayTileReply(sock, node)
		Packages up a tile message for socket communication.
		"""
		if len(node.tiles_on_displays) == 0:
			sock.Failure( SockWrapper.UNKNOWNPARAM, "server doesn't have display tiles!" )
			return
		tiles = "%d " % len(node.tiles_on_displays)
		for i in range(len(node.tiles_on_displays)):
			tile = node.tiles_on_displays[i]
			tiles += "%d %d %d %d %d" % tile
			if i != len(node.tiles_on_displays) - 1:
				tiles += ","
		sock.Success( tiles )

	def do_newclients( self, sock, args ):
		"""do_clients(sock, args)
		Sends the list of new clients to a server."""
		sock.Success( "1 tcpip 0" )

	def do_clients( self, sock, args ):
		"""do_clients(sock, args)
		Sends the list of clients to a server."""
		if sock.node == None or not isinstance(sock.node,CRNetworkNode):
			sock.Failure( SockWrapper.UNKNOWNSERVER, "You can't ask for clients without telling me what server you are!" )
			return
		total_clients = len(sock.node.clients) + len(sock.node.file_clients)
		clients = "%d " % total_clients
		for i in range(len(sock.node.clients)):
			(spu, protocol) = sock.node.clients[i]
			clients += "%s %d" % (protocol, spu.ID)
			if i != total_clients-1:
				clients += ','
		for i in range(len(sock.node.file_clients)):
			fname = sock.node.file_clients[i]
			clients += "%s %d" % (fname, -1)
			if i-len(sock.node.clients) != total_clients-1:
				clients += ','
		sock.Success( clients )
	
	def do_reset( self, sock, args ):
		"""do_reset(sock, args)
		Resets the mothership to its initial state."""
		for node in self.nodes:
			node.spokenfor = 0
			node.spusloaded = 0
			node.crut_spokenfor = 0
		# respawn auto-start nodes
		for cb in CR.startupCallbacks:
			cb(self)
		spawner = CRSpawner( self.nodes )
		spawner.start()
		sock.Success( "Server Reset" );

	def do_rank( self, sock, args ):
		"""do_rank( sock, args )
		Retrieves the node's rank and sends it on the socket."""
		if sock.node == None:
			sock.Failure( SockWrapper.UNKNOWNSERVER, "Identify yourself!" )
			return
		if not sock.node.config.has_key( 'rank' ):
			sock.Failure( SockWrapper.UNKNOWNPARAM, "Node didn't say what it's rank is." )
			return
		sock.Success( sock.node.config['rank'] )

	def do_quit( self, sock, args ):
		"""do_quit(sock, args)
		Disconnects from clients."""
		sock.Success( "Bye" )
		self.ClientDisconnect( sock )

	def propagate_quit(self, daughter, args):
		daughter.Send("quit")
		line = daughter.readline() # ignore since we're going away

	def do_logperf( self, sock, args ):
		"""do_logperf(sock, args)
		Logs Data to a logfile."""
		CROutput("%s" % args)
		sock.Success( "Dumped" )

	def do_gettilelayout( self, sock, args ):
		"""Call the user's tile layout function and return the resulting
		list of tiles."""
		if sock.SPUid == -1:
			sock.Failure( SockWrapper.UNKNOWNSPU,
							  "You can't ask for a new tile layout without "
							  "telling me what (tilesort) SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		if spu.name != "tilesort":
			# this is bad
			sock.Success("0")
			return
		argv = string.split(args)
		assert len(argv) == 2
		muralWidth = int(argv[0])
		muralHeight = int(argv[1])
		fn = getattr(spu, "layoutFunction" )
		if fn == None:
			# XXX return failure?
			sock.Success("0")
			return
		tiles = fn(muralWidth, muralHeight)
		# reformat the tiles list into a string
		result = str(len(tiles)) + " "
		for t in tiles:
			result += "%d %d %d %d %d, " % (t[0], t[1], t[2], t[3], t[4])
		if result[-2:] == ", ":
			result = result[:-2]  # remove trailing ", "
		assert len(result) < 8000  # see limit in getNewTiling in tilesort SPU
		sock.Success( result )
		return

	def do_getstatus( self, sock, args ):
		"""Returns status information for the mothership.
		
		The first argument determines what information is sent:
		0 [or nonexistent] - Send simple summary info back.
		1 - Send detailed summary info back.
		2 - Send node count.
		3 n attr - Send attr value for node n.
		
		# Not yet implemented, intended for GUI use
		4 [n] - Send node setup information for node n [if n not given, is sent for all nodes].
		5 [n] - Send node status information for node n [if n not given, is sent for all nodes].
		"""
		
		args = string.split(args)
		
		node_types = [ [CRNetworkNode, "network node"],
							[CRUTServerNode, "CRUT server node"],
							[CRUTProxyNode, "CRUT proxy node"],
							[CRApplicationNode, "application node"] ]
		
		TYPE, NAME, COUNT, CONNECTED = 0, 1, 2, 3
		
		result = ""
		
		if len(args) == 0 or (args[0] == "0" or args[0] == "1"):
			total_connected = 0
			
			# Set the node type count and node type connected counts to 0
			for node_type in node_types:
				node_type.append(0)
				node_type.append(0)
			
			for node in self.nodes:
				for node_type in node_types:
					if isinstance(node, node_type[TYPE]):
						node_type[COUNT] = node_type[COUNT] + 1
						if node.spokenfor:
							node_type[CONNECTED] = node_type[CONNECTED] + 1
							total_connected = total_connected + 1
			
			result = "%d nodes, %d connected" % (len(self.nodes), total_connected)
			
			is_detailed = (len(args) > 0 and args[0] == "1")
			
			for node_type in node_types:
				if node_type[COUNT]:
					if is_detailed:
						result = result + ("<br>  %sS:" % string.upper(node_type[NAME])) + self.__create_detailed_summary(node_type[TYPE])
					else:
						result = result + "<br>  %d %ss, %d connected" % (node_type[COUNT], node_type[NAME], node_type[CONNECTED])
			
		elif args[0] == "2":
			result = "%d" % len(self.nodes)
			
		elif args[0] == "3":
			if len(args) < 2:
				sock.Failure(SockWrapper.INVALIDPARAM, "getstatus usage: 3 n attr - Get attr value for node n.")
				return
			
			try:
				attr = getattr(self.nodes[int(args[1])], args[2])
			except AttributeError:
				sock.Failure(SockWrapper.INVALIDPARAM, "Invalid node attribute: %s" % args[2])
				return
			except IndexError:
				sock.Failure(SockWrapper.INVALIDPARAM, "Node index out of range: %s" % args[1])
				return
			except ValueError:
				sock.Failure(SockWrapper.INVALIDPARAM, "Invalid node index: %s" % args[1])
				return
			
			result = MakeString(attr)
		
		sock.Success( result )

	def __create_detailed_summary ( self, node_type ):
		"""Creates a detailed summary string."""
		
		result = ""
		
		for node_num in range(len(self.nodes)):
			node = self.nodes[node_num]
			if isinstance(node, node_type):
				if node.spokenfor:
					result = result + "<br>    %s[%d] has connected" % (node.host, node_num)
				else:
					result = result + "<br>    %s[%d] has NOT connected" % (node.host, node_num)
		
		return result

	def do_daughter( self, sock, args ):
		# This socket has identified itself as a daughter socket.  She
		# wants the node graph in reply; and in the future, she'll receive
		# propagated commands.
		self.daughters.append(sock)

		# Make a copy of the node graph; we'll munge the copy up
		# before sending it along.
		copyCR = copy.copy(self)

		# The daughter has no interest in any of our connections;
		# and the mothership has already autostarted everything
		copyCR.all_sockets = []
		copyCR.wrappers = {}
		copyCR.daughters = []
		copyCR.mother = None
		copyCR.enable_autostart = None

		# Package the copy of CR up with the other necessary globals
		globals = { }
		globals['cr'] = copyCR
		globals['allSPUs'] = allSPUs
		globals['dynamicHosts'] = dynamicHosts

		# Send them to the daughtership
		pickledGlobals = pickle.dumps(globals)
		# The current interface only sends one line at a time
		lines = pickledGlobals.splitlines()
		for line in lines:
			sock.MoreComing(line)
		sock.Success("hi sweetheart")

	def ProcessRequest( self, sock_wrapper ):
		"""ProcessRequest(sock_wrapper)
		Handles an incoming request, mapping it to an appropriate
		do_* function."""
		try:
			line = sock_wrapper.readline()
			CRDebug("Processing mothership request: \"%s\"" % line)
		except:
			# Client is gone.  Make sure it isn't a special client
			if sock_wrapper in self.daughters:
				CRDebug("Daughter quit without saying goodbye?  How rude!")
				self.daughters.remove(sock_wrapper)
				self.ClientDisconnect( sock_wrapper )
				return
			elif sock_wrapper == self.mother:
				Fatal("Mother is gone; so am I.")
			else:
				CRDebug( "Client quit without saying goodbye?  How rude!" )
				self.ClientDisconnect( sock_wrapper )
				return

		words = string.split( line )
		if len(words) == 0: 
			self.ClientError( sock_wrapper,
							  SockWrapper.NOTHINGTOSAY, "Request was empty?" )
			#sock_wrapper.Failure( SockWrapper.NOTHINGTOSAY, "Request was empty?" )
			return
		command = string.lower( words[0] )
		arguments = string.join( words[1:] )

		try:
			fn = getattr(self, 'do_%s' % command )
		except AttributeError:
			sock_wrapper.Failure( SockWrapper.UNKNOWNCOMMAND, "Unknown command: %s" % command )
			return

		# If we have any daughterships, and if we need to propagate this request
		# to them, do so.
		if len(self.daughters) > 0:
			try:
				fn = getattr(self, 'propagate_%s' % command)
				for daughter in self.daughters:
					fn(daughter, arguments)
			except AttributeError:
				# Don't have to propagate this request.
				pass

		# If we're actually a daughtership, we'll have a valid mother attribute,
		# and we might have to propagate this command upward.
		if self.mother:
			try:
				fn = getattr(self, 'tattle_%s' % command)
				fn(self.mother, arguments)
			except AttributeError:
				# Don't have to propagate this one.
				pass

		# Here, we're in the normal case: finish executing the command locally.
		# (We have to propagate before executing locally because some commands,
		# like "quit", will stop us from continuing.)
		fn( sock_wrapper, arguments)

class CRDaughtership:
	def __init__( self, mother = None ):
		self.mother = None
		self.cr = None

		# Poor little lost daughtership, looking for her mother
		if mother == None:
			if os.environ.has_key('CRMOTHER'):
				mother = os.environ['CRMOTHER']
		if mother == None:
			CRInfo("Lost daughter - using localhost on default port")
			motherHost = 'localhost'
			motherPort = 10000
		else:
			colon = string.find(mother, ':')
			if colon >= 0:
				motherHost = mother[0:loc-1]
				try:
					motherPort = int(mother[colon+1:])
				except:
					CRInfo("Illegal port number %s, using default" % mother[colon+1:])
					motherPort = 10000
			else:
				motherHost = mother
				motherPort = 10000
				
		# Try all available socket types to reach our mothership
		motherSocket = None
		for res in socket.getaddrinfo(motherHost, motherPort, socket.AF_UNSPEC, socket.SOCK_STREAM, 0, socket.AI_PASSIVE):
			(af, socktype, proto, canonname, sa) = res

			try:
				motherSocket = socket.socket( af, socktype, proto )
			except:
				CRDebug( "Couldn't create socket of family %u, trying another one" % af );
				motherSocket = None
				continue

			try:
				motherSocket.connect( sa )
			except:
				s.close()
				CRDebug( "Couldn't connect to mothership at %s:%d" % (motherHost, motherPort));
				motherSocket = None
				continue

		if motherSocket == None:
			Fatal("Could not open connection to mothership at %s:%d" % (motherHost, motherPort))

		self.mother = SockWrapper(motherSocket)
		# Tell the mothership that we are a daughtership, so that we'll
		# receive propagated commands.
		self.mother.Send("daughter")

		# The response will come in multiple lines
		done = False
		pickledGlobals = ""
		while not done:
			reply = self.mother.readline()
			words = string.split(reply, None, 1)
			if len(words) == 0:
				Fatal("Mothership returned empty reply?")
			if words[0] == "200":
				# Done
				done = 1
			elif words[0] == "100":
				# More coming
				pickledGlobals = pickledGlobals + words[1] + "\n"
			else:
				Fatal("Mothership doesn't recognize its daughter [%s]" % words[0])
		
		# By now we've got the whole pickle.  See if we can unpickle it.
		try:
			globals = pickle.loads(pickledGlobals)
		except:
			Fatal("Could not unpickle Cr globals")

		# Unpack all the globals that we were given
		try:
			global allSPUs, dynamicHosts
			self.cr = globals['cr']
			allSPUs = globals['allSPUs']
			dynamicHosts = globals['dynamicHosts']
		except KeyError, badKey:
			Fatal("Globals were missing the key '%s'" % badKey)
				
		# Modify the CR configuration so it knows it has a mother.
		# Some commands will then automatically propagate to the
		# mothership from us.
		self.cr.mother = self.mother

		# The mothership should already have taken care of eliminating
		# other things we don't want to see (like the mothership's own
		# sockets, etc.), so we should be ready to go.

	def Go(self):
		# Just tell the Chromium configuration to go.  It should be
		# all set up and ready.
		self.cr.Go()

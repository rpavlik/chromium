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

Other internal functions/classes:
    Conf:           Sets a key/values list of configuration values.
    CRNode:         Base class that defines a node in the SPU graph
    CRDebug:        Used to print out debugging messages.
    Fatal:          Used to print out a debugging messages and exit.
    MakeString:     Converts a Python object to a string.
    Conf:           Sets a name/value list into a configuration.
    SockWrapper:    Internal convience class for handling sockets
"""

import sys, string, types, traceback, re

import socket
import select

def CRDebug( str ):
	"""CRDebug(str)
	Prints debugging message to stderr."""
	print >> sys.stderr, str

allSPUs = {}

def Fatal( str ):
	"""Fatal(str)
	Prints debugging message to stderr and skeddadles."""
	print >> sys.stderr, str
	sys.exit(-1)

def MakeString( x ):
	"""MakeString(x)
	Converts an object to a string"""
	if type(x) == types.StringType: return x
	return repr(x)

def Conf( config, key, *values ):
	"""Conf(config, key, *values)
	Sets the configuration in "config" of "key" to a list
	containing "values"."""
	config[key] = map( MakeString, values )

class SPU:
	"""Main class that defines a Stream Processing Unit.

	public functions:

	    Conf:	Sets a key/value list in this SPU's configuration
	    AddServer:  Tells a client node where to find its server.
	"""
	def __init__( self, name ):
                """SPU(name)
		Creates a SPU with the given "name"."""
		self.name = name
		self.config = {}
		self.clientargs = []
		self.servers = []

	def Conf( self, key, *values ):
                """Conf(key,*values)
		Sets a key/value list in this SPU's configuration"""
		Conf( self.config, key, *values )

	def __add_server( self, node, url ):
		self.servers.append( (node, url) )

	def AddServer( self, node, protocol='tcpip', port=7000 ):
        	"""AddServer(node, protocol='tcpip', port=7000)
                Tells a client node where to find its server."""
		node.Conf( 'port', port )
		if protocol == 'tcpip':
			self.__add_server( node, "%s://%s:%d" % (protocol,node.ipaddr,port) )
		else:
			self.__add_server( node, "%s://%s:%d" % (protocol,node.host,port) )
		node.AddClient( self, protocol )

class CRNode:
	"""Base class that defines a node in the SPU graph

	public functions:

	    AddSPU:	Adds a SPU to the front of the SPU chain.
	    SPUDir:	Sets the directory SPUs start in.
	"""
	SPUIndex = 0

	def __init__( self, host ):
	    	"""CRNode(host)
		Creates a node on the given "host"."""
		self.host = host
		if (host == 'localhost'):
			self.host = socket.gethostname()
		self.ipaddr = socket.gethostbyname(self.host)

		# unqualify the hostname if it is already that way.
		# e.g., turn "foo.bar.baz" into "foo"
		period = self.host.find( "." )
		if period != -1:
			self.host = self.host[:period]

		self.SPUs = []
		self.spokenfor = 0
		self.spusloaded = 0
		self.config = {}
		self.accept_wait = None
		self.connect_wait = None
		self.alias = host

	def Alias( self, name ):
		self.alias = name
	
	def AddSPU( self, spu ):
	    	"""AddSPU(spu)
		Adds the given SPU to the front of the SPU chain."""
		self.SPUs.append( spu )
		spu.ID = CRNode.SPUIndex
		spu.node = self

		CRNode.SPUIndex += 1
		allSPUs[spu.ID] = spu

	def SPUDir( self, dir ):
	    	"""SPUDir(dir)
		Sets the directory that SPUs start in."""
		self.config['SPUdir'] = dir

class CRNetworkNode(CRNode):
	"""Sub class of CRNode that defines a node in the SPU graph that
	handles incoming and outgoing network traffic.

	public functions:

	    Conf:	Sets a key/value list in this node's configuration
	    AddClient:	Adds a client to the list of clients.
	    AddTile:	Adds a tile to the list of tiles
	"""
	def __init__( self, host='localhost' ):
	    	"""CRNetworkNode(host='localhost')
		Creates a network node for the given "host"."""
		CRNode.__init__(self,host)
		self.clients = []
		self.tiles = []

	def Conf( self, key, *values ):
		"""Conf(key, *values)
		Sets a key/value list in this node's configuration"""
		Conf( self.config, key, *values )

	def AddClient( self, node, protocol ):
		"""AddClient(node, protocol)
		Adds a client node, communicating with "protocol", to the
		list of clients."""
		self.clients.append( (node,protocol) )

	def AddTile( self, x, y, w, h ):
		"""AddTile(x, y, w, h)
		Defines a tile with the given geometry to be used by a
		tilesort SPU.
		"""
		self.tiles.append( (x,y,w,h) )

class CRApplicationNode(CRNode):
	"""Sub class of CRNode that defines the start of the the SPU graph.

	public functions:

	    SetApplication:	Sets the application that generates the OpenGL.
	    StartDir:		Sets the starting directory of the app.
	    ClientDLL:		Sets the DLL of the client.
	"""
	AppID = 0

	def __init__(self,host='localhost'):
	    	"""CRApplicationNode(host='localhost')
		Creates an application node for the given "host"."""
		CRNode.__init__(self,host)
		self.id = CRApplicationNode.AppID
		CRApplicationNode.AppID += 1;

	def SetApplication( self, app ):
		"""SetApplication(name)
		Sets the name of the application that's run."""
		self.application = app

	def StartDir( self, dir ):
		"""SetApplication(dir)
		Sets the directory the application starts in."""
		self.config['startdir'] = dir

	def ClientDLL( self, dir ):
		"""ClientDLL(dir)
		XXX Don't know what this does (ahern)
		Sets the client DLL."""
		self.config['clientdll'] = dir

class SockWrapper:
	"Internal convenience class for handling sockets"
	NOERROR = 200
	UNKNOWNHOST = 400
	NOTHINGTOSAY = 401
	UNKNOWNCOMMAND = 402
	UNKNOWNSPU = 403
	UNKNOWNPARAM = 404
	UNKNOWNSERVER = 405
	UNKNOWNPROTOCOL = 406

	def __init__(self, sock):
		self.sock = sock
		self.file = sock.makefile( "r" )
		self.SPUid = -1
		self.node = None
		self.accept_wait = None
		self.connect_wait = None

	def readline( self ):
		return self.file.readline()
	
	def Send(self, str):
		self.sock.send( str + "\n" )

	def Reply(self, code, str=None):
		tosend = `code`
		if str != None:
			tosend += " " + str
		self.Send( tosend )
		CRDebug( 'Replying (%d): "%s"' % ( code, str ) )

	def Success( self, msg ):
		self.Reply( SockWrapper.NOERROR, msg )

class CR:
	"""Main class that controls the mothership

        Most of the mothership network communication takes the form of
        incoming strings that the mothership responds to with answer
        strings.  The do_* functions handle this communication language.

	public functions:
	    AddNode:	Adds a node to the SPU graph.
	    MTU: 	Sets the maximum communication buffer size.
	    Go:		Starts the ball rolling.
	    AllSPUConf: Adds the key/values list to all SPUs' configuration.

	internal functions:
            ProcessRequest:     Handles an incoming request, mapping it to
                                an appropriate do_* function.
	    do_acceptrequest:	Accepts the given socket.
	    do_clientdll: 	Sends the clientdll configuration down.
	    do_clients: 	Sends the list of clients to a server.
	    do_connectrequest:	Connects the given socket.
	    do_faker:		Maps a faker app to an ApplicationNode.
	    do_mtu:
	    do_namedspuparam:   Sends the given SPU parameter.
	    do_opengldll:	Identifies the application node in the graph.
	    do_quit: 		Disconnects from clients.
	    do_reset: 		Resets the mothership to its initial state.
	    do_server:		Identifies the server in the graph.
	    do_serverids:	Sends the list of server IDs.
	    do_serverparam: 	Sends the given server parameter.
	    do_servers: 	Sends the list of servers.
	    do_servertiles: 	Sends the defined tiles for a server.
	    do_setspuparam: 	Sets a SPU parameter.
	    do_spudir: 		Sends the spudir configuration down.
	    do_spu:		Identifies a SPU.
	    do_spuparam:	Sends the given SPU (or global) parameter.
	    do_startdir:	Sends the startup directory to a SPU or server.
	    do_tiles:		Sends the defined tiles for a SPU.
	    tileReply: 		Packages up a tile message for socket communication.
	    ClientDisconnect: 	Disconnects from a client
	    ClientError:	Sends an error message on the given socket.
	"""
	def __init__( self ):
		self.nodes = []
		self.all_sockets = []
		self.wrappers = {}
		self.mtu = 1024*1024
		self.allSPUConf = []

	def AddNode( self, node ):
		"""AddNode(node)
		Adds a node to the SPU graph."""
		self.nodes.append( node )
	
	def MTU( self, mtu ):
		"""MTU(size)
		Sets the maximum buffer size allowed in communication
		between SPUs."""
		self.mtu = mtu;

	def AllSPUConf( self, regex, key, *values ):
		"""AllSPUConf(regex, key, *values)
		XXX Greg, is this right? (ahern)
		Adds the key/values list to all SPUs' configuration."""
		self.allSPUConf.append( (regex, key, map( MakeString, values) ) )
		
	def Go( self, PORT=10000 ):
		"""Go(PORT=10000)
		Starts the ball rolling.
		This starts the mothership's event loop."""
		print >> sys.stderr, "This is Chromium, Version ALPHA"
		try:
			HOST = ""
			try:
				s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
			except:
				Fatal( "Couldn't create socket" );

 			try:
				s.setsockopt( socket.SOL_SOCKET, socket.SO_REUSEADDR, 1 )
			except:
				Fatal( "Couldn't set the SO_REUSEADDR option on the socket!" )

			try:
				s.bind( (HOST, PORT) )
			except:
				Fatal( "Couldn't bind to port %d" % PORT );

			try:
				s.listen(100)
			except:
				Fatal( "Couldn't listen!" );
	
			self.all_sockets.append(s)
	
			while 1:
				ready = select.select( self.all_sockets, [], [], 0.1 )[0]
				for sock in ready:
					if sock == s:
						conn, addr = s.accept()
						self.wrappers[conn] = SockWrapper(conn)
						self.all_sockets.append( conn )
					else:
						self.ProcessRequest( self.wrappers[sock] )
		except KeyboardInterrupt:
			try:
				for sock in self.all_sockets:
					sock.shutdown(2)
					sock.close( )
			except:
				pass
			print >> sys.stderr, "\n\nThank you for using Chromium!"
		except:
			print >> sys.stderr, "\n\nMOTHERSHIP EXCEPTION!  TERRIBLE!"
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
		if (protocol == 'tcpip'):
			(p, hostname, port_str, endianness_str) = connect_info
			port = int(port_str)
			endianness = int(endianness_str)
			for server_sock in self.wrappers.values():
				if server_sock.accept_wait != None:
					(server_hostname, server_port, server_endianness) = server_sock.accept_wait
					if server_hostname == hostname:
						sock.Success( "%d" % server_endianness )
						server_sock.Success( "" )
						return
					else:
						CRDebug( "not accepting from %s" % server_hostname )
			sock.connect_wait = (hostname, port, endianness)
		elif (protocol == 'gm'):
			(p, hostname, port_str, node_id_str, port_num_str, endianness_str) = connect_info
			port = int(port_str)
			node_id = int(node_id_str)
			port_num = int(port_num_str)
			endianness = int(endianness_str)
			for server_sock in self.wrappers.values():
				if server_sock.accept_wait != None:
						(server_hostname, server_port, server_node_id, server_port_num, server_endianness) = server_sock.accept_wait
						if server_hostname == hostname:
							sock.Success( "%d %d %d" % (server_node_id, server_port_num, server_endianness) )
							server_sock.Success( "%d %d" % (node_id, port_num) )
							return
			sock.connect_wait = (hostname, port, node_id, port_num, endianness)
		else:
			self.ClientError( sock, SockWrapper.UNKNOWNPROTOCOL, "Never heard of protocol %s" % protocol )

	def do_acceptrequest( self, sock, args ):
		"""do_acceptrequest(sock, args)
		Accepts the given socket."""
		accept_info = args.split( " " )
		protocol = accept_info[0]
		if protocol == 'tcpip':
			(p, hostname, port_str, endianness_str) = accept_info
			hostname = socket.gethostbyname(hostname)
			port = int(port_str)
			endianness = int(endianness_str)
			for client_sock in self.wrappers.values():
				if client_sock.connect_wait != None:
					(client_hostname, client_port, client_endianness) = client_sock.connect_wait
					if client_hostname == hostname:
						sock.Success( "" )
						client_sock.Success( "%d" % endianness ) 
						return
					else:
						CRDebug( "not accepting from %s" % client_hostname )
						
			sock.accept_wait = (hostname, port, endianness)
		elif protocol == 'gm':
			(p, hostname, port_str, node_id_str, port_num_str, endianness_str) = accept_info
			port = int(port_str)
			node_id = int(node_id_str)
			port_num = int(port_num_str)
			endianness = int(endianness_str)
			for client_sock in self.wrappers.values():
				if client_sock.connect_wait != None:
					(client_hostname, client_port, client_node_id, client_port_num, client_endianness) = client_sock.connect_wait
					if client_hostname == hostname:
						sock.Success( "%d %d" % (client_node_id, server_port_num) )
						client_sock.Success( "%d %d %d" % (node_id, port_num, endianness) )
						return
			sock.accept_wait = (hostname, port, node_id, port_num, endianness)
		else:
			self.ClientError( sock, SockWrapper.UNKNOWNPROTOCOL, "Never heard of protocol %s" % protocol )

	def do_faker( self, sock, args ):
		"""do_faker(sock, args)
		Maps the incoming "faker" app to a previously-defined node."""
		for node in self.nodes:
			if string.lower(node.host) == string.lower(args) and not node.spokenfor:
				if isinstance(node,CRApplicationNode):
					node.spokenfor = 1
					sock.node = node
					sock.Success( "%d %s" % (node.id, node.application) )
					return
		self.ClientError( sock, SockWrapper.UNKNOWNHOST, "Never heard of faker host %s" % args )

	def do_clientdll( self, sock, args ):
		"""do_clientdll(sock, args)
		Retrieves the clientdll configuration and sends it on the socket."""
		if sock.node == None or not isinstance(sock.node,CRApplicationNode):
			self.ClientError( sock, SockWrapper.UNKNOWNSERVER, "You're not a faker!" )
			return
		if not sock.node.config.has_key( 'clientdll' ):
			sock.Reply( SockWrapper.UNKNOWNPARAM, "Faker didn't say where it was." )
			return
		sock.Success( sock.node.config['clientdll'] )

	def do_spudir( self, sock, args ):
		"""do_spudir(sock, args)
		Retrieves the spudir configuration and sends it on the socket."""
		if sock.node == None:
			self.ClientError( sock, SockWrapper.UNKNOWNSERVER, "Identify yourself!" )
			return
		if not sock.node.config.has_key( 'SPUdir' ):
			sock.Reply( SockWrapper.UNKNOWNPARAM, "Node didn't say where the SPUs were." )
			return
		sock.Success( sock.node.config['SPUdir'] )

	def do_server( self, sock, args ):
		"""do_server(sock, args)
		Identifies the server in the graph. """
		for node in self.nodes:
			if string.lower(node.host) == string.lower(args) and not node.spokenfor:
				if isinstance(node,CRNetworkNode):
					node.spokenfor = 1
					node.spusloaded = 1
					sock.node = node

					spuchain = "%d" % len(node.SPUs)
					for spu in node.SPUs:
						spuchain += " %d %s" % (spu.ID, spu.name)
					sock.Success( spuchain )
					return
		self.ClientError( sock, SockWrapper.UNKNOWNHOST, "Never heard of server host %s" % args )

	def do_opengldll( self, sock, args ):
		"""do_opengldll(sock, args)
		XXX Is this right??!  Not sure. (ahern)
		Identifies the application node in the graph."""
		(id_string, hostname) = args.split( " " )
		app_id = int(id_string)
		for node in self.nodes:
			if isinstance(node,CRApplicationNode):
				if ((app_id == -1 and hostname == node.host) or node.id == app_id) and not node.spusloaded:
					node.spusloaded = 1
					spuchain = "%d" % len(node.SPUs)
					for spu in node.SPUs:
						spuchain += " %d %s" % (spu.ID, spu.name)
					sock.Success( spuchain )
					sock.node = node
					return
		self.ClientError( sock, SockWrapper.UNKNOWNHOST, "Never heard of OpenGL DLL for application %d" % app_id )

	def do_spu( self, sock, args ):
		"""do_spu(sock, args)
		Identifies a SPU."""
		try:
			spuid = int(args)
		except:
			self.ClientError( sock, SockWrapper.UNKNOWNSPU, "Bogus SPU name: %s" % args )
			return
		if not allSPUs.has_key( spuid ):
			self.ClientError( sock, SockWrapper.UNKNOWNSPU, "Never heard of SPU %d" % spuid )
			return
		sock.SPUid = spuid
		sock.Success( "Hello, %s SPU!" % allSPUs[spuid].name )

	def do_spuparam( self, sock, args ):
		"""do_spuparam(sock, args)
		Sends the given SPU (or global) parameter."""
		if sock.SPUid == -1:
			self.ClientError( sock, SockWrapper.UNKNOWNSPU, "You can't ask for SPU parameters without telling me what SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		if not spu.config.has_key( args ):
			# Okay, there's no specific parameter for the SPU.  Try the global SPU configurations.
			for (regex, key, values) in self.allSPUConf:
				if args == key and re.search( regex, spu.name ) != -1:
					response = values
					break
			else:
				sock.Reply( SockWrapper.UNKNOWNPARAM,
                                            "SPU %d (%s) doesn't have param %s"
                                            % (sock.SPUid,
                                               allSPUs[sock.SPUid].name,
                                               args) )
                                return
		else:
			response = spu.config[args]
		print >> sys.stderr, "responding with args = " + `response`
		sock.Success( string.join( response, " " ) )	

	def do_namedspuparam( self, sock, args ):
		"""do_namedspuparam(sock, args)
		Sends the given SPU parameter."""
		# XXX New- Greg should review (BrianP)
##		if sock.SPUid == -1:
##			self.ClientError( sock, SockWrapper.UNKNOWNSPU, "You can't ask for SPU parameters without telling me what SPU id you are!" )
##			return
		params = args.split( " " )
		spuid = int(params[0])
		key = params[1]
		#print >> sys.stderr, "Get named spu param id is %d" % spuid
		#print >> sys.stderr, "Get named spu param key is " + key
		spu = allSPUs[spuid]
		if spu.config.has_key( key ):
			response = spu.config[key]
		else:
			# Okay, there's no specific parameter for the SPU.  Try the global SPU configurations.
			sock.Reply( SockWrapper.UNKNOWNPARAM, "SPU %d doesn't have param %s" % (spuid, args) )
			return
		print >> sys.stderr, "responding with args = " + `response`
		sock.Success( string.join( response, " " ) )	


	def do_setspuparam( self, sock, args):
		"""do_setspuparam(sock, args)
		Sets a SPU parameter."""
		# XXX this should be reviewed by Greg  (BrianP)
		if sock.SPUid == -1:
			self.ClientError( sock, SockWrapper.UNKNOWNSPU, "You can't set SPU parameters without telling me what SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		#spu = self.allSPUConf
		params = args.split( " ", 1 )
		key = params[0]
		value = params[1]
		# print >> sys.stderr, "Setting key " + key + " to " + value
		spu.config[key] = [value]
		print >> sys.stderr, "responding with OK"
		sock.Success( "OK" )

	def do_serverparam( self, sock, args ):
		"""do_serverparam(sock, args)
		Sends the given server parameter."""
		if sock.node == None or not isinstance(sock.node,CRNetworkNode):
			self.ClientError( sock, SockWrapper.UNKNOWNSERVER, "You can't ask for server parameters without telling me what server you are!" )
			return
		if not sock.node.config.has_key( args ):
			sock.Reply( SockWrapper.UNKNOWNPARAM, "Server doesn't have param %s" % (args) )
			return
		sock.Success( string.join( sock.node.config[args], " " ) )	

	def do_servers( self, sock, args ):
		"""do_servers(sock, args)
		Sends the list of servers."""
		if sock.SPUid == -1:
			self.ClientError( sock, SockWrapper.UNKNOWNSPU, "You can't ask for servers without telling me what SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		if len(spu.servers) == 0:
			sock.Reply( SockWrapper.UNKNOWNPARAM, "SPU %d doesn't have servers!" % (sock.SPUid) )
			return

		servers = "%d " % len(spu.servers)
		for i in range(len(spu.servers)):
			(node, url) = spu.servers[i]
			servers += "%s" % (url)
			if i != len(spu.servers) -1:
				servers += ','
		sock.Success( servers )

	def do_serverids( self, sock, args ):
		"""do_serverids(sock, args)
		Sends the list of server IDs.
		XXX How is this different from do_servers? (ahern)
		"""
		# XXX this might only be temporary (BrianP)
		if sock.SPUid == -1:
			self.ClientError( sock, SockWrapper.UNKNOWNSPU, "You can't ask for server ids without telling me what SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		if len(spu.servers) == 0:
			sock.Reply( SockWrapper.UNKNOWNPARAM, "SPU %d doesn't have servers!" % (sock.SPUid) )
			return

		servers = "%d " % len(spu.servers)
		for i in range(len(spu.servers)):
			(node, url) = spu.servers[i]
			servers += "%d" % (node.SPUs[0].ID)
			if i != len(spu.servers) - 1:
				servers += ' '
		sock.Success( servers )

	def do_tiles( self, sock, args ):
		"""do_tiles(sock, args)
		Sends the defined tiles for a SPU."""
		if sock.SPUid == -1:
			self.ClientError( sock, SockWrapper.UNKNOWNSPU, "You can't ask for tiles without telling me what SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		if len(spu.servers) == 0:
			sock.Reply( SockWrapper.UNKNOWNPARAM, "SPU %d doesn't have servers!" % (sock.SPUid) )
			return
		server_num = int(args)
		if server_num < 0 or server_num >= len(spu.servers):
			self.ClientError( sock, SockWrapper.UNKNOWNSERVER, "SPU %d doesn't have a server numbered %d" % (sock.SPUid, server_num) )
		(node, url) = spu.servers[server_num]
		self.tileReply( sock, node )

	def do_servertiles( self, sock, args ):
		"""do_servertiles(sock, args)
		Sends the defined tiles for a server."""
		if sock.node == None or not isinstance(sock.node,CRNetworkNode):
			self.ClientError( sock, SockWrapper.UNKNOWNSERVER, "You can't ask for tiles without telling me what server you are!" )
			return
		self.tileReply( sock, sock.node )

	def tileReply( self, sock, node ):
		"""tileReply(sock, node)
		Packages up a tile message for socket communication."
		"""
		if len(node.tiles) == 0:
			sock.Reply( SockWrapper.UNKNOWNPARAM, "server doesn't have tiles!" )
			return
		tiles = "%d " % len(node.tiles)
		for i in range(len(node.tiles)):
			tile = node.tiles[i]
			tiles += "%d %d %d %d" % tile
			if i != len(node.tiles) - 1:
				tiles += ","
		sock.Success( tiles )

	def do_clients( self, sock, args ):
		"""do_clients(sock, args)
		Sends the list of clients to a server."""
		if sock.node == None or not isinstance(sock.node,CRNetworkNode):
			self.ClientError( sock, SockWrapper.UNKNOWNSERVER, "You can't ask for clients without telling me what server you are!" )
			return
		clients = "%d " % len(sock.node.clients)
		for i in range(len(sock.node.clients)):
			(spu, protocol) = sock.node.clients[i]
			clients += "%s %d" % (protocol, spu.ID)
			if i != len(sock.node.clients) -1:
				clients += ','
		sock.Success( clients )
	
	def do_reset( self, sock, args ):
		"""do_reset(sock, args)
		Resets the mothership to its initial state."""
		for node in self.nodes:
			node.spokenfor = 0
			node.spusloaded = 0
		sock.Success( "Server Reset" );

	def do_mtu( self, sock, args ):
		"""do_mtu(sock, args)
		XXX Is this basically a NOP?  (ahern)
		Does nothing but indicate success."""
		sock.Success( `self.mtu` )

	def do_quit( self, sock, args ):
		"""do_quit(sock, args)
		Disconnects from clients."""
		sock.Success( "Bye" )
		self.ClientDisconnect( sock )

	def do_startdir( self, sock, args ):
		"""do_startdir(sock, args)
		Sends the startup directory to a SPU or server."""
		if not sock.node:
			self.ClientError( sock_wrapper, SockWrapper.UNKNOWNHOST, "Can't ask me where to start until you tell me who you are." )
		if not sock.node.config.has_key( "startdir" ):
			sock.Success( "." )
		else:
			sock.Success( sock.node.config['startdir'] )

	def ProcessRequest( self, sock_wrapper ):
		"""ProcessRequest(sock_wrapper)
		Handles an incoming request, mapping it to an appropriate
		do_* function."""
		try:
			line = string.strip(sock_wrapper.readline())
			print >> sys.stderr, "Processing mothership request: \"%s\"" % line
		except:
			CRDebug( "Client blew up?" )
			self.ClientDisconnect( sock_wrapper )
			return
		words = string.split( line )
		if len(words) == 0: 
			self.ClientError( sock_wrapper, SockWrapper.NOTHINGTOSAY, "Request was empty?" )
			return
		command = string.lower( words[0] )
                #CRDebug("command = " + command)
		try:
			fn = getattr(self, 'do_%s' % command )
		except AttributeError:
			self.ClientError( sock_wrapper, SockWrapper.UNKNOWNCOMMAND, "Unknown command: %s" % command )
			return
		fn( sock_wrapper, string.join( words[1:] ) )

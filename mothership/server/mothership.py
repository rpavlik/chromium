import sys, string, types

from socket import *
from select import *

def CRDebug( str ):
	print >> sys.stderr, str

allSPUs = {}

def Conf( config, key, *values ):
	def makestr( x ):
		if type(x) == types.StringType: return x
		return repr(x)
	config[key] = map( makestr, values )

class SPU:
	def __init__( self, name ):
		self.name = name
		self.config = {}
		self.clientargs = []

	def Conf( self, key, *values ):
		Conf( self.config, key, *values )

	def Server( self, node, protocol='tcpip', port=7000 ):
		self.Conf( 'server', "%s://%s:%d" % (protocol,node.host,port) )
		node.AddClient( self, protocol )

class CRNode:
	SPUIndex = 0

	def __init__( self, host ):
		self.host = host
		self.SPUs = []
		self.spokenfor = 0
		self.spusloaded = 0
		self.config = {}
	
	def AddSPU( self, spu ):
		self.SPUs.append( spu )
		spu.ID = CRNode.SPUIndex
		spu.node = self

		CRNode.SPUIndex += 1
		allSPUs[spu.ID] = spu

class CRNetworkNode(CRNode):
	def __init__( self, host ):
		CRNode.__init__(self,host)
		self.clients = []

	def Conf( self, key, *values ):
		Conf( self.config, key, *values )

	def AddClient( self, node, protocol ):
		self.clients.append( (node,protocol) )

class CRApplicationNode(CRNode):
	def SetApplication( self, app ):
		self.application = app

	def StartDir( self, dir ):
		self.config['startdir'] = dir

	def ClientDLL( self, dir ):
		self.config['clientdll'] = dir

	def SPUDir( self, dir ):
		self.config['SPUdir'] = dir

class SockWrapper:
	NOERROR = 200
	UNKNOWNHOST = 400
	NOTHINGTOSAY = 401
	UNKNOWNCOMMAND = 402
	UNKNOWNSPU = 403
	UNKNOWNPARAM = 404
	UNKNOWNSERVER = 405

	def __init__(self, sock):
		self.sock = sock
		self.file = sock.makefile( "r" )
		self.SPUid = -1
		self.node = None

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
	def __init__( self ):
		self.nodes = []
		self.all_sockets = []
		self.wrappers = {}

	def AddNode( self, node ):
		self.nodes.append( node )

	def Go( self ):
		try:
			HOST = ""
			PORT = 10000
			s = socket( AF_INET, SOCK_STREAM )
			s.bind( (HOST, PORT) )
			s.listen(100)
	
			self.all_sockets.append(s)
	
			while 1:
				ready = select( self.all_sockets, [], [], 0.1 )[0]
				for sock in ready:
					if sock == s:
						conn, addr = s.accept()
						self.wrappers[conn] = SockWrapper(conn)
						self.all_sockets.append( conn )
					else:
						self.ProcessRequest( self.wrappers[sock] )
		except:
			try:
				s.shutdown( 2 )
			except:
				pass
			print >> sys.stderr, "\n\nThank you for using Chromium!"

	def ClientError( self, sock_wrapper, code, msg ):
		sock_wrapper.Reply( code, msg )
		self.ClientDisconnect( sock_wrapper )

	def ClientDisconnect( self, sock_wrapper ):
		self.all_sockets.remove( sock_wrapper.sock )
		try:
			sock_wrapper.sock.shutdown( 2 )
		except:
			pass

	def do_faker( self, sock, args ):
		for node in self.nodes:
			if node.host == args and not node.spokenfor:
				if isinstance(node,CRApplicationNode):
					node.spokenfor = 1
					sock.node = node
					sock.Success( node.application )
					return
		self.ClientError( sock, SockWrapper.UNKNOWNHOST, "Never heard of faker host %s" % args )

	def do_clientdll( self, sock, args ):
		if sock.node == None or not isinstance(sock.node,CRApplicationNode):
			self.ClientError( sock, SockWrapper.UNKNOWNSERVER, "You're not a faker!" )
			return
		if not sock.node.config.has_key( 'clientdll' ):
			sock.Reply( SockWrapper.UNKNOWNPARAM, "Faker didn't say where it was." )
			return
		sock.Success( sock.node.config['clientdll'] )

	def do_spudir( self, sock, args ):
		if sock.node == None or not isinstance(sock.node,CRApplicationNode):
			self.ClientError( sock, SockWrapper.UNKNOWNSERVER, "You're not a faker!" )
			return
		if not sock.node.config.has_key( 'SPUdir' ):
			sock.Reply( SockWrapper.UNKNOWNPARAM, "Faker didn't say where it was." )
			return
		sock.Success( sock.node.config['SPUdir'] )

	def do_server( self, sock, args ):
		for node in self.nodes:
			if node.host == args and not node.spokenfor:
				if isinstance(node,CRNetworkNode):
					node.spokenfor = 1
					node.spusloaded = 1
					sock.node = node

					spuchain = "%d" % len(node.SPUs)
					for spu in node.SPUs:
						spuchain += " %d %sspu" % (spu.ID, spu.name)
					sock.Success( spuchain )
					return
		self.ClientError( sock, SockWrapper.UNKNOWNHOST, "Never heard of server host %s" % args )

	def do_opengldll( self, sock, args ):
		for node in self.nodes:
			if node.host == args and not node.spusloaded:
				if isinstance(node,CRApplicationNode):
					spuchain = "%d" % len(node.SPUs)
					for spu in node.SPUs:
						spuchain += " %d %sspu" % (spu.ID, spu.name)
					sock.Success( spuchain )
					return
		self.ClientError( sock, SockWrapper.UNKNOWNHOST, "Never heard of OpenGL DLL host %s" % args )

	def do_spu( self, sock, args ):
		spuid = int(args)
		if not allSPUs.has_key( spuid ):
			self.ClientError( sock, SockWrapper.UNKNOWNSPU, "Never heard of SPU %d" % spuid )
			return
		sock.SPUid = spuid
		sock.Success( "Hello, SPU!" )

	def do_spuparam( self, sock, args ):
		if sock.SPUid == -1:
			self.ClientError( sock, SockWrapper.UNKNOWNSPU, "You can't ask for SPU parameters without telling me what SPU id you are!" )
			return
		spu = allSPUs[sock.SPUid]
		if not spu.config.has_key( args ):
			sock.Reply( SockWrapper.UNKNOWNPARAM, "SPU %d doesn't have param %s" % (sock.SPUid, args) )
			return
		print "responding with args = " + `spu.config[args]`
		sock.Success( string.join( spu.config[args], " " ) )	

	def do_serverparam( self, sock, args ):
		if sock.node == None or not isinstance(sock.node,CRNetworkNode):
			self.ClientError( sock, SockWrapper.UNKNOWNSERVER, "You can't ask for Server parameters without telling me what server you are!" )
			return
		if not sock.node.config.has_key( args ):
			sock.Reply( SockWrapper.UNKNOWNPARAM, "Server doesn't have param %s" % (args) )
			return
		sock.Success( string.join( sock.node.config[args], " " ) )	

	def do_clients( self, sock, args ):
		if sock.node == None or not isinstance(sock.node,CRNetworkNode):
			self.ClientError( sock, SockWrapper.UNKNOWNSERVER, "You can't ask for clients without telling me what server you are!" )
			return
		clients = "%d " % len(sock.node.clients)
		for i in range(len(sock.node.clients)):
			(spu, protocol) = sock.node.clients[i]
			print >> sys.stderr, `sock.node.clients[i]`
			clients += "%s %d" % (protocol, spu.ID)
			if i != len(sock.node.clients) -1:
				clients += ','
		sock.Success( clients )
	
	def do_reset( self, sock, args ):
		for node in self.nodes:
			node.spokenfor = 0
			node.spusloaded = 0
		sock.Success( "Server Reset" );

	def do_quit( self, sock, args ):
		sock.Success( "Bye" )
		self.ClientDisconnect( sock )

	def do_startdir( self, sock, args ):
		if not sock.node:
			self.ClientError( sock_wrapper, SockWrapper.UNKNOWNHOST, "Can't ask me where to start until you tell me who you are." )
		if not sock.node.config.has_key( "startdir" ):
			sock.Success( "." )
		else:
			sock.Success( sock.node.config['startdir'] )

	def ProcessRequest( self, sock_wrapper ):
		try:
			line = string.strip(sock_wrapper.readline())
			print "Got a line: " + line
		except:
			CRDebug( "Client blew up?" )
			self.ClientDisconnect( sock_wrapper )
			return
		words = string.split( line )
		if len(words) == 0: 
			self.ClientError( sock_wrapper, SockWrapper.NOTHINGTOSAY, "Nothing to say?" )
			return
		command = string.lower( words[0] )
		print "command = " + command
		try:
			fn = getattr(self, 'do_%s' % command )
		except AttributeError:
			self.ClientError( sock_wrapper, SockWrapper.UNKNOWNCOMMAND, "Unknown command: %s" % command )
			return
		fn( sock_wrapper, string.join( words[1:] ) )

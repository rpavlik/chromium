import string

from socket import *
from select import *

class SPU:
	def __init__( self, name ):
		self.name = name
		self.config = {}

	def Conf( self, key, *values ):
		self.config[key] = values

class CRNode:
	def __init__( self, host ):
		self.host = host
		self.SPUs = []
	
	def AddSPU( self, spu ):
		self.SPUs.append( spu )

class CRNetworkNode(CRNode):
	pass

class CRApplicationNode(CRNode):
	def SetApplication( self, app ):
		self.application = app

class SockWrapper:
	ITSALLGOOD = 200

	def __init__(self, sock):
		self.sock = sock
		self.file = sock.makefile( "r" )

	def readline( self ):
		return self.file.readline()
	
	def Send(self, str):
		self.sock.send( str + "\n" )

	def Reply(self, code, str=None):
		tosend = `code`
		if str != None:
			tosend += " " + str
		self.Send( tosend )


class CR:
	def __init__( self ):
		self.nodes = []
		self.all_sockets = []

	def AddNode( self, node ):
		self.nodes.append( node )

	def Go( self ):
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
					print "accepted from " + `addr`
					self.all_sockets.append( conn )
				else:
					print "got some data!"
					self.ProcessRequest( SockWrapper(sock) )

	def ClientDisconnect( self, sock_wrapper ):
		self.all_sockets.remove( sock_wrapper.sock )
		sock_wrapper.sock.shutdown( 2 )

	def do_host( self, sock, string ):
		for node in self.nodes:
			if node.host == string:
				sock.cur_node = node
				return
		sock.Send( "Never heard of host %s" % string )
		self.ClientDisconnect( sock )

	def ProcessRequest( self, sock_wrapper ):
		line = sock_wrapper.readline()
		words = string.split( line )
		if len(words) == 0: 
			self.ClientDisconnect( sock_wrapper )
			return
		command = string.lower( words[0] )
		print "command = " + command
		try:
			fn = getattr(self, 'do_%s' % command )
		except AttributeError:
			sock_wrapper.Send( "Unknown command: %s" % command )
			self.ClientDisconnect( sock_wrapper )
			return
		fn( sock_wrapper, string.join( words[1:] ) )

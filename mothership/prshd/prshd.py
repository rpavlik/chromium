# WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 
#
# This is probably the most insecure piece of software ever written.  It's a
# poor man's RSHd, and by poor I mean "running Windows".  I'm sure there's a
# legit Windows RSHd out there somewhere, but this is easier.
#
# Basically, you hit this guy's port, type a string, and it gets executed.
# Wow.  Don't run this unless you're SURE you know what you're doing.
#
#     - Greg Humphreys
#
# WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING


import sys, string, os
from socket import *
from select import *

HOST = ""
PORT = 6969
s = socket( AF_INET, SOCK_STREAM )
s.bind( (HOST, PORT) )
s.listen(100)

all_sockets = []
all_sockets.append(s)

def ProcessRequest( sock ):
	file = sock.makefile( "r" )
	try:
		command = file.readline()
		print >> sys.stderr, "Okay, the command is " + command
		os.system( command )
	except:
		pass

	all_sockets.remove( sock )
	sock.shutdown(2)

while 1:
	ready = select( all_sockets, [], [], 0.1 )[0]
	for sock in ready:
		if sock == s:
			conn, addr = s.accept()
			all_sockets.append( conn )
		else:
			ProcessRequest( sock )

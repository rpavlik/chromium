import sys,os,os.path,re

countfile = open( "model_info", "w" )
face_re = re.compile( r"element face (?P<faces>\d+)$" )

def CountFile( path ):
	print >> sys.stderr, "Counting file %s..." % file
	fp = open( path, "r" )
	while 1:
		line = fp.readline()
		match = face_re.match( line )
		if match != None:
			faces = int(match.group( "faces" ))
			os.system ("plybbox %s > /tmp/the_count" % path )
			bbox = open( "/tmp/the_count", "r" ).readlines()[0].strip()
			print >> countfile, "%s %d %s" % (path, faces, bbox)
			fp.close()

			return

for dir in os.listdir("."):
	if os.path.isdir( dir ):
		for sdir in os.listdir( dir ):
			if os.path.isdir( dir ):
				for file in os.listdir( "%s/%s" % (dir,sdir) ):
					if file.endswith( ".ply" ):
						CountFile( "%s/%s/%s" % (dir,sdir,file) )

import sys,string

print "char lowercase[256] = {"

NUM_COLS = 8

count = 0
for num in range(256):
	if count%NUM_COLS == 0:
		sys.stdout.write( '\t' )
	the_char = chr(num);
	if num != 255:
		print ("'\%03o'," % ord(string.lower(the_char))),
	else:
		print ("'\%03o'" % ord(string.lower(the_char))),
	count += 1
	if count%NUM_COLS == 0:
		print ""

print "};"

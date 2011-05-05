import subprocess
import os
pathToGrabber = "../grab.mk"
def getVariable(var):
	proc = subprocess.Popen(["make", "--silent", "-f", pathToGrabber, "GETVAR", "VARNAME=%s" % var], stdout = subprocess.PIPE)
	return proc.communicate()[0].strip()
def getVariableList(var):
	return [item.strip() for item in getVariable(var).split()]

#print getTargetOutput("LIBRARY")
#print getTargetOutput("FILES")
#print getTargetOutput("PROGRAM")

def getSources():
	#return ["%s.c" % fn.strip() for fn in getTargetOutput("FILES").split()]
	return ["%s.c" % fn.strip() for fn in getVariableList("FILES")]

def getTarget():
	ret = {}
	prog = getTargetOutput("PROGRAM").strip()
	lib = getTargetOutput("LIBRARY").strip()
	if len(prog) > 0:
		return prog, "add_executable(%s ${SOURCES})\n" % prog
	elif len(lib) > 0:
		if getTargetOutput("SHARED") == "1":
			return lib, "add_library(%s SHARED ${SOURCES})\n" % lib
		else:
			return lib, "add_library(%s STATIC ${SOURCES})\n" % lib
	else:
		raise Exception("Not a lib or a program!")
with open("CMakeLists.txt","w") as cmake:
	cmake.write("set(SOURCES\n")
	cmake.writelines(["\t%s\n" % fn for fn in getSources()])
	cmake.write(")\n")
	
	target, targetline = getTarget()
	cmake.write(targetline)


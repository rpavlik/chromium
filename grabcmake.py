import subprocess
import os
pathToGrabber = os.path.abspath("grab.mk")
def getVariable(var):
	proc = subprocess.Popen(["make", "--silent", "-f", pathToGrabber, "GETVAR", "VARNAME=%s" % var], stdout = subprocess.PIPE)
	return proc.communicate()[0].strip()
def getVariableList(var):
	return [item.strip() for item in getVariable(var).split()]

#print getTargetOutput("LIBRARY")
#print getTargetOutput("FILES")
#print getTargetOutput("PROGRAM")

def getSources():
	return ["%s.c" % fn.strip() for fn in getVariableList("FILES")]

def getTarget():
	ret = {}
	prog = getVariable("PROGRAM")
	lib = getVariable("LIBRARY")
	if len(prog) > 0:
		return prog, "add_executable(%s ${SOURCES})\n" % prog
	elif len(lib) > 0:
		if getVariable("SHARED") == "1":
			return lib, "add_library(%s SHARED ${SOURCES})\n" % lib
		else:
			return lib, "add_library(%s STATIC ${SOURCES})\n" % lib
	else:
		return None, ""

def doDirectory(sourcedir):
	print sourcedir
	os.chdir(sourcedir)
	with open("CMakeLists.txt","w") as cmake:
		target, targetline = getTarget()
		if target is not None:
			cmake.write("set(SOURCES\n")
			cmake.writelines(["\t%s\n" % fn for fn in getSources()])
			cmake.write(")\n")

			cmake.write(targetline)

			libs = [ lib.replace("-l", "") for lib in getVariableList("LIBRARIES") ]
			if len(libs) > 0:
				cmake.write("target_link_libraries(%s %s)\n" % (target, " ".join(libs)))

		dirs = getVariableList("SUBDIRS")
		if len(dirs) > 0:
			cmake.writelines(["add_subdirectory(%s)\n" % dirname for dirname in dirs])


sourcedirs = [ dirpath for (dirpath, dirnames, filenames) in os.walk(os.path.abspath(".")) if "Makefile" in filenames]
for sourcedir in sourcedirs:
	doDirectory(sourcedir)

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
		return prog, "add_executable", ""
	elif len(lib) > 0:
		if getVariable("SHARED") == "1":
			return lib, "add_library", "SHARED"
		else:
			return lib, "add_library", "STATIC"
	else:
		return None, "", ""

def doDirectory(sourcedir):
	print sourcedir
	os.chdir(sourcedir)
	with open("CMakeLists.txt","w") as cmake:

		cmake.write("include_directories(.)\n")
		target, targetcommand, targettype = getTarget()
		if target is not None:
			generated = getVariableList("PRECOMP")
			if len(generated) > 0:

				cmake.write("set(GENERATED\n")
				cmake.writelines(["\t%s\n" % fn for fn in generated])
				cmake.write(")\n")
				cmake.write("# TODO: generate these files!\n\n\n")

			cmake.write("set(SOURCES\n")
			cmake.writelines(["\t%s\n" % fn for fn in getSources() if fn not in generated])
			cmake.writelines(["\t${CMAKE_CURRENT_BINARY_DIR}/%s\n" % fn for fn in generated])
			cmake.write(")\n")



			cmake.write("%s(%s %s ${SOURCES})\n" % (targetcommand, target, targettype))

			libs = [ lib.replace("-l", "") for lib in getVariableList("LIBRARIES") ]
			if len(libs) > 0:
				cmake.write("target_link_libraries(%s %s)\n" % (target, " ".join(libs)))

			cmake.write("target_link_libraries(%s ${EXTRA_LIBS})\n" % target)
			cmake.write("""install(TARGETS %s
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)\n""" % target)
			for copy in getVariableList("LIB_COPIES"):
				copytarget = "%s_%s_copy" % (copy, target)
				cmake.write("%s(%s %s ${SOURCES})\n" % (targetcommand, copytarget, targettype))
				if len(libs) > 0:
					cmake.write("target_link_libraries(%s %s)\n" % (copytarget, " ".join(libs)))
				cmake.write("target_link_libraries(%s ${EXTRA_LIBS})\n" % copytarget)
				cmake.write("""install(TARGETS %s
	LIBRARY DESTINATION lib COMPONENT runtime
	ARCHIVE DESTINATION lib COMPONENT dev
	RUNTIME DESTINATION bin COMPONENT runtime)\n""" % copytarget)
		dirs = getVariableList("SUBDIRS")
		if len(dirs) > 0:
			cmake.writelines(["add_subdirectory(%s)\n" % dirname for dirname in dirs])


sourcedirs = [ dirpath for (dirpath, dirnames, filenames) in os.walk(os.path.abspath(".")) if "Makefile" in filenames]
for sourcedir in sourcedirs:
	doDirectory(sourcedir)

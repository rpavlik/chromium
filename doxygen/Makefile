default: full

all: full
	
%.tag: %.doxy
	doxygen $<

FULL = \
	include.doxy \
	include_state.doxy \
	glapi_parser.doxy \
	util.doxy \
	mothership.doxy \
	mothership_server.doxy \
	spu_loader.doxy \
	packer.doxy \
	state_tracker.doxy \
	unpacker.doxy \
	dlm.doxy \
	app_faker.doxy \
	opengl_stub.doxy \
	crserverlib.doxy \
	crserver.doxy \
	render.doxy \
	pack.doxy \
	binaryswap.doxy \
	array.doxy \
	expando.doxy \
	print.doxy \
	nop.doxy \
	perf.doxy \
	motionblur.doxy \
	wet.doxy \
	error.doxy \
	tester.doxy \
	passthrough.doxy \
	tilesort.doxy \
	feedback.doxy \
	options.doxy \
	readback.doxy \
	replicate.doxy \
	template.doxy \
	crutapi.doxy \
	crutclientapi.doxy  \
	crutproxy.doxy \
	crutserver.doxy  \
	packertest.doxy \
	threadtest.doxy \
	bboxtest.doxy \
	tiletest.doxy \
	psubmit_crut.doxy \
	psubmit.doxy \
	packperf.doxy \
	dlisttest.doxy \
	arbfraglight.doxy 

full: dirs $(FULL:.doxy=.tag)
#	$(foreach FILE,$(FULL),doxygen $(FILE);)

dirs:
	-mkdir spu
	-mkdir progs

clobber clean:
	rm -rf $(FULL:.doxy=) spu progs
	rm -rf *.tag

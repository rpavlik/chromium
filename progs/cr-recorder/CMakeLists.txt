include_directories(.)
set(SOURCES
	cr-recorder.c
	recorderdialog.c
	recorderdialogimpl.c
	moc_recorderdialog.c
	moc_recorderdialogimpl.c
	moc_cr-recorder.c
)
add_executable(cr-recorder  ${SOURCES})
target_link_libraries(cr-recorder crutil crpacker)
target_link_libraries(cr-recorder ${EXTRA_LIBS})

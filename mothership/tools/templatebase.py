# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul


class TemplateBase:
	"""Base class for all template modules."""

	def __init__(self):
		pass
		
	def Name(self):
		"""Return name of this template class."""
		raise "Template.Name() is pure virtual!"

	def Create(self, parentWindow, mothership):
		"""Create nodes/spus/etc for this template."""
		raise "Template.Create() is pure virtual!"

	def Validate(self, mothership):
		"""Return 1 if mothership is an instance of this template type,
		0 otherwise."""
		raise "Template.Validate() is pure virtual!"

	def Edit(self, parentWindow, mothership):
		"""Open editor window and edit the mothership config"""
		raise "Template.Edit() is pure virtual!"

	def Read(self, mothership, fileHandle):
		"""Read template from file"""
		raise "Template.Read() is pure virtual!"

	def Write(self, mothership, fileHandle):
		raise "Template.Write() is pure virtual!"

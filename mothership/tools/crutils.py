"""Chromium config tool utility functions"""

import re

def MakeHostname(format, number):
	"""Return a hostname generated from a format string and number."""
	# find the hash characters first
	p = re.search("#+", format)
	if not p:
		return format
	numHashes = p.end() - p.start()
	numDigits = len(str(number))
	# start building result string
	result = format[0:p.start()]
	# insert padding zeros as needed
	while numHashes > numDigits:
		result += "0"
		numHashes -= 1
	# append the number
	result += str(number)
	# append rest of format string
	result += format[p.end():]
	return result

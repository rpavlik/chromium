# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.
#
# Authors:
#   Brian Paul

"""
Tile layout functions

This is a collection of functions for computing tile layouts for
tilesort configs, specifically image reassembly configs.
There are lots of ways this can be done - only a few are presented here.

These functions are used by the graphical config tool and you may use
them in your own tilesort configuration file.

The basic task is to divide a mural into tiles while assigning each
tile to a "server".  Ideally, one wants the mural's tiles to be
uniformly distributed among the servers so that we get good load
balancing.

The user might request a particular number of rows and columns of
tiles.  Then we'll have to compute the tile sizes to cover the mural.

Or, the user might request a particular tile size.  Then we'll have to
compute how many rows and columns of tiles are needed to cover the mural.

These functions all take the same parameters:
  muralWidth, muralHeight = the mural size, in pixels
  numServers = number of servers available
  tileWidth, tileHeight = the desired tile size (may be zero)
  tileRows, tileCols = desired number of rows and cols of tiles (may be zero)
Note: only the tileWidth/Height _OR_ tileRows/Cols values can be zero,
not both.

The functions return a list of tuples of the form
(server, x, y, width, height) which describes the position and size of
each tile as well as its assigned server.
"""


def __CheckArgs(muralWidth, muralHeight, numServers, tileWidth, tileHeight, tileRows, tileCols):
	"""Helper function for layout functions."""
	# error checking
	assert muralWidth > 0
	assert muralHeight > 0
	assert numServers > 0
	# XXX Note Python integer division below (use // someday?)
	if tileRows > 0 and tileCols > 0:
		# we're told how many rows and columns to use
		assert tileWidth >= 0
		assert tileHeight >= 0
		# if tile size is zero, compute it now
		if tileWidth == 0:
			tileWidth = muralWidth / tileCols
		if tileHeight == 0:
			tileHeight = muralHeight / tileRows
		fixedTileSize = 0
	else:
		# we're told the tile size, compute rows and columns
		assert tileWidth > 0
		assert tileHeight > 0
		tileCols = (muralWidth + tileWidth - 1) / tileWidth
		tileRows = (muralHeight + tileHeight - 1) / tileHeight
		fixedTileSize = 1

	return (tileWidth, tileHeight, tileRows, tileCols, fixedTileSize)


def LayoutRaster(muralWidth, muralHeight, numServers, tileWidth, tileHeight, tileRows, tileCols):
	"""Layout tiles in a simple top-to-bottom, left-to-right raster order."""
	# check args, and compute missing values
	(tileWidth, tileHeight, tileRows, tileCols, fixedTileSize) = __CheckArgs(muralWidth, muralHeight, numServers, tileWidth, tileHeight, tileRows, tileCols)

	# layout tiles now
	tiles = []
	for i in range(tileRows):
		for j in range(tileCols):
			server = (i * tileCols + j) % numServers
			x = j * tileWidth
			y = i * tileHeight
			if i < tileRows - 1 or fixedTileSize:
				h = tileHeight
			else:
				h = muralHeight - (tileRows - 1) * tileHeight
			if j < tileCols - 1 or fixedTileSize:
				w = tileWidth
			else:
				w = muralWidth - (tileCols - 1) * tileWidth
			tiles.append( (server, x, y, w, h) )
	return tiles


def LayoutZigZag(muralWidth, muralHeight, numServers, tileWidth, tileHeight, tileRows, tileCols):
	"""Layout tiles in a simple top-to-bottom order, alternating between
	left-to-right and right-to-left with each row."""
	# check args, and compute missing values
	(tileWidth, tileHeight, tileRows, tileCols, fixedTileSize) = __CheckArgs(muralWidth, muralHeight, numServers, tileWidth, tileHeight, tileRows, tileCols)

	# layout tiles now
	tiles = []
	for i in range(tileRows):
		for j in range(tileCols):
			if i % 2 == 1:
				# odd row
				jj = tileCols - j - 1
				server = (i * tileCols + jj) % numServers
			else:
				# even row
				server = (i * tileCols + j) % numServers
			x = j * tileWidth
			y = i * tileHeight
			if i < tileRows - 1 or fixedTileSize:
				h = tileHeight
			else:
				h = muralHeight - (tileRows - 1) * tileHeight
			if j < tileCols - 1 or fixedTileSize:
				w = tileWidth
			else:
				w = muralWidth - (tileCols - 1) * tileWidth
			tiles.append( (server, x, y, w, h) )
	return tiles


def LayoutSpiral(muralWidth, muralHeight, numServers, tileWidth, tileHeight, tileRows, tileCols):
	"""Layout tiles starting in the center of the mural and winding outward
	in a spiral."""
	# check args, and compute missing values
	(tileWidth, tileHeight, tileRows, tileCols, fixedTileSize) = __CheckArgs(muralWidth, muralHeight, numServers, tileWidth, tileHeight, tileRows, tileCols)

	curRow = (tileRows - 1) / 2
	curCol = (tileCols - 1) / 2
	radius = 0
	march = 0
	colStep = 0
	rowStep = -1
	serv = 0
	tiles = []
	while 1:
		assert ((rowStep == 0 and colStep != 0) or
				(rowStep != 0 and colStep == 0))
		if (curRow >= 0 and curRow < tileRows and
			curCol >= 0 and	curCol < tileCols):

			server = serv % numServers

			# compute tile position and size
			x = curCol * tileWidth
			y = curRow * tileHeight
			if curCol < tileCols - 1 or fixedTileSize:
				w = tileWidth
			else:
				w = muralWidth - (tileCols - 1) * tileWidth
			if curRow < tileRows - 1 or fixedTileSize:
				h = tileHeight
			else:
				h = muralHeight - (tileRows - 1) * tileHeight

			# save this tile
			tiles.append( (server, x, y, w, h) )

			# check if we're done
			if (len(tiles) >= tileRows * tileCols):
				# all done
				break
		serv += 1
		# advance to next space
		march += 1
		if march < radius:
			# step in current direction
			curRow += rowStep
			curCol += colStep
			pass
		else:
			# change direction
			if colStep == 1 and rowStep == 0:
				# transition right -> down
				colStep = 0
				rowStep = 1
			elif colStep == 0 and rowStep == 1:
				# transition down -> left
				colStep = -1
				rowStep = 0
				radius += 1
			elif colStep == -1 and rowStep == 0:
				# transition left -> up
				colStep = 0
				rowStep = -1
			else:
				# transition up -> right
				assert colStep == 0
				assert rowStep == -1
				colStep = 1
				rowStep = 0
				radius += 1
			#endif
			march = 0
			curRow += rowStep
			curCol += colStep
		#endif
	#endwhile
	return tiles


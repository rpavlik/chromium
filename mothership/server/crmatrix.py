# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

"""
4x4 Matrix routines
"""

import math



class CRMatrix:
	"""4x4 matrix transformation class.
	Methods mimic OpenGL's matrix functions."""
	# Implementation note: the array elements are stored in a 16-element
	# list in column-major order, to match OpenGL.
	def __init__( self ):
		"""Initialize matrix to the identity."""
		self.LoadIdentity()

	def Set( self, row, col, value ):
		"""Set a matrix element."""
		self.values[col * 4 + row] = value

	def Get( self, row, col ):
		"""Get a matrix element."""
		return self.values[col * 4 + row]

	def ToList( self ):
		"""Return list of 16 elements in the matrix (column-major order)."""
		return self.values

	def ToString( self ):
		"""Return string representation of the matrix."""
		s = "[ "
		for v in self.values:
			s = s + "%.3f" % v + ", "
		s = s + " ]"
		#s = repr(self.values)
		return s

	def Print( self ):
		"""Pretty print the matrix."""
		print "[ %.3f %.3f %.3f %.3f ]" % (self.values[0], self.values[4],
										   self.values[8], self.values[12])
		print "[ %.3f %.3f %.3f %.3f ]" % (self.values[1], self.values[5],
										   self.values[9], self.values[13])
		print "[ %.3f %.3f %.3f %.3f ]" % (self.values[2], self.values[6],
										   self.values[10], self.values[14])
		print "[ %.3f %.3f %.3f %.3f ]" % (self.values[3], self.values[7],
										   self.values[11], self.values[15])
		
	def LoadIdentity( self ):
		"""Set matrix to identity."""
		self.values = [ 1.0, 0.0, 0.0, 0.0,
						0.0, 1.0, 0.0, 0.0,
						0.0, 0.0, 1.0, 0.0,
						0.0, 0.0, 0.0, 1.0 ]

	def Load( self, m ):
		"""Replace my matrix with m (a list of 16 floats)."""
		assert len(m) == 16
		self.values = m[:]  # copy

	def Multiply( self, m ):
		"""Post multiply by matrix m (a list of 16 floats)."""
		assert len(m) == 16
		prod = range(16)  # initial list product
		for i in range(0, 4):
			ai0 = self.Get(i, 0)
			ai1 = self.Get(i, 1)
			ai2 = self.Get(i, 2)
			ai3 = self.Get(i, 3)
			prod[0*4+i] = ai0 * m[0*4+0] + ai1 * m[0*4+1] + ai2 * m[0*4+2] + ai3 * m[0*4+3]
			prod[1*4+i] = ai0 * m[1*4+0] + ai1 * m[1*4+1] + ai2 * m[1*4+2] + ai3 * m[1*4+3]
			prod[2*4+i] = ai0 * m[2*4+0] + ai1 * m[2*4+1] + ai2 * m[2*4+2] + ai3 * m[2*4+3]
			prod[3*4+i] = ai0 * m[3*4+0] + ai1 * m[3*4+1] + ai2 * m[3*4+2] + ai3 * m[3*4+3]
		self.values = prod

	def Scale( self, sx, sy, sz ):
		"""Post multiply matrix with a scaling matrix."""
		s = CRMatrix()
		s.Set(0, 0, sx)
		s.Set(1, 1, sy)
		s.Set(2, 2, sz)
		self.Multiply(s.ToList())

	def Translate( self, tx, ty, tz ):
		"""Post multiply matrix with a translation matrix."""
		t = CRMatrix()
		t.Set(0, 3, tx)
		t.Set(1, 3, ty)
		t.Set(2, 3, tz)
		self.Multiply(t.ToList())

	def XRotate( self, angle ):
		"""Post multiply matrix with an X-axis rotation."""
		s = math.sin(angle * math.pi / 180.0)
		c = math.cos(angle * math.pi / 180.0)
		r = CRMatrix()
		r.Set(1, 1, c)
		r.Set(1, 2, -s)
		r.Set(2, 1, s)
		r.Set(2, 2, c)
		self.Multiply(r.ToList())

	def YRotate( self, angle ):
		"""Post multiply matrix with an Y-axis rotation."""
		s = math.sin(angle * math.pi / 180.0)
		c = math.cos(angle * math.pi / 180.0)
		r = CRMatrix()
		r.Set(0, 0, c)
		r.Set(0, 2, s)
		r.Set(2, 0, -s)
		r.Set(2, 2, c)
		self.Multiply(r.ToList())

	def ZRotate( self, angle ):
		"""Post multiply matrix with an Z-axis rotation."""
		s = math.sin(angle * math.pi / 180.0)
		c = math.cos(angle * math.pi / 180.0)
		r = CRMatrix()
		r.Set(0, 0, c)
		r.Set(0, 1, -s)
		r.Set(1, 0, s)
		r.Set(1, 1, c)
		self.Multiply(r.ToList())

	def Rotate( self, angle, x, y, z ):
		"""Post multiply matrix by a rotation matrix of <angle> degrees
		about the axis specified by (x, y, z)."""
		r = CRMatrix()
		mag = math.sqrt(x * x + y * y + z * z)
		if mag != 0.0:
			x /= mag
			y /= mag
			z /= mag
		s = math.sin(angle * math.pi / 180.0)
		c = math.cos(angle * math.pi / 180.0)
		r.Set(0, 0, x * x * (1.0 - c) + c)
		r.Set(0, 1, x * y * (1.0 - c) - z * s)
		r.Set(0, 2, x * z * (1.0 - c) + y * s)
		r.Set(1, 0, y * x * (1.0 - c) + z * s)
		r.Set(1, 1, y * y * (1.0 - c) + c)
		r.Set(1, 2, y * z * (1.0 - c) - x * s)
		r.Set(2, 0, x * z * (1.0 - c) - y * s)
		r.Set(2, 1, y * z * (1.0 - c) + x * s)
		r.Set(2, 2, z * z * (1.0 - c) + c)
		self.Multiply(r.ToList())

	def Lookat( self, eyeX, eyeY, eyeZ, toX, toY, toZ, upX = 0.0, upY = 1.0, upZ = 0.0 ):
		"""Post multiply matrix with a viewing transformation."""
		# make Z vector
		z = range(3) # list of 3
		z[0] = eyeX - toX
		z[1] = eyeY - toY
		z[2] = eyeZ - toZ
		mag = math.sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2])
		if mag != 0.0:
			z[0] /= mag
			z[1] /= mag
			z[2] /= mag

		# make Y vector
		y = range(3)
		y[0] = upX
		y[1] = upY
		y[2] = upZ

		# compute x = y cross z
		x = range(3)
		x[0] =  y[1] * z[2] - y[2] * z[1]
		x[1] = -y[0] * z[2] + y[2] * z[0]
		x[2] =  y[0] * z[1] - y[1] * z[0]

		# recompute y = z cross x
		y[0] =  z[1] * x[2] - z[2] * x[1]
		y[1] = -z[0] * x[2] + z[2] * x[0]
		y[2] =  z[0] * x[1] - z[1] * x[0]

		# normalize x
		mag = math.sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2])
		if mag != 0.0:
			x[0] /= mag
			x[1] /= mag
			x[2] /= mag

		# normalize y
		mag = math.sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2])
		if mag != 0.0:
			y[0] /= mag
			y[1] /= mag
			y[2] /= mag

		# rotation matrix constructed from basis vectors
		r = CRMatrix()
		r.Set(0, 0, x[0])
		r.Set(0, 1, x[1])
		r.Set(0, 2, x[2])
		r.Set(1, 0, y[0])
		r.Set(1, 1, y[1])
		r.Set(1, 2, y[2])
		r.Set(2, 0, z[0])
		r.Set(2, 1, z[1])
		r.Set(2, 2, z[2])
		self.Multiply(r.ToList())
		self.Translate(-eyeX, -eyeY, -eyeZ)

	def Frustum( self, left, right, bottom, top, near, far ):
		"""Post multiply matrix with a perspective projection."""
		f = CRMatrix()
		f.Set(0, 0, 2.0 * near / (right - left))
		f.Set(0, 1, 0.0)
		f.Set(0, 2, (right + left) / (right - left))
		f.Set(0, 3, 0.0)
		f.Set(1, 0, 0.0)
		f.Set(1, 1, 2.0 * near / (top - bottom))
		f.Set(1, 2, (top + bottom) / (top - bottom))
		f.Set(1, 3, 0.0)
		f.Set(2, 0, 0.0)
		f.Set(2, 1, 0.0)
		f.Set(2, 2, -(far + near) / (far - near))
		f.Set(2, 3, -2.0 * far * near / (far - near))
		f.Set(3, 0, 0.0)
		f.Set(3, 1, 0.0)
		f.Set(3, 2, -1.0)
		f.Set(3, 3, 0.0)
		self.Multiply(f.ToList())

	def Ortho( self, left, right, bottom, top, near, far ):
		"""Post multiply matrix with an orthographic projection."""
		o = CRMatrix()
		o.Set(0, 0, 2.0 / (right - left))
		o.Set(0, 1, 0.0)
		o.Set(0, 2, 0.0)
		o.Set(0, 3, -(right + left) / (right - left))
		o.Set(1, 0, 0.0)
		o.Set(1, 1, 2.0 / (top - bottom))
		o.Set(1, 2, 0.0)
		o.Set(1, 3, -(top + bottom) / (top - bottom))
		o.Set(2, 0, 0.0)
		o.Set(2, 1, 0.0)
		o.Set(2, 2, -2.0 / (far - near))
		o.Set(2, 3, -(far + near) / (far - near))
		o.Set(3, 0, 0.0)
		o.Set(3, 1, 0.0)
		o.Set(3, 2, 0.0)
		o.Set(3, 3, 1.0)
		self.Multiply(o.ToList())

	def DecomposeProjection(self):
		"""Examine a projection matrix and return the Frustum() or Ortho()
		parameters which were used to create it.
		Return value is a tuple with the values
		(left, right, bottom, top, near, far, isPerspective)."""
		if self.Get(3,3) == 0.0:
			# perspective projection
			x = self.Get(0,0)
			y = self.Get(1,1)
			a = self.Get(0,2)
			b = self.Get(1,2)
			c = self.Get(2,2)
			d = self.Get(2,3)
			near = -d / (1.0 - c)
			far = (c - 1.0) * near / (c + 1.0)
			left = near * (a - 1.0) / x
			right = 2.0 * near / x + left
			bottom = near * (b - 1.0) / y
			top = 2.0 * near / y + bottom
			isPerspective = 1
		else:
			# orthographic
			x = self.Get(0,0)
			y = self.Get(1,1)
			z = self.Get(2,2)
			a = self.Get(0,3)
			b = self.Get(1,3)
			c = self.Get(2,3)
			right = -(a - 1.0) / x
			left = right - 2.0 / x
			top = -(b - 1.0) / y
			bottom = top - 2.0 / y
			far = (c - 1.0) / z
			near = far + 2.0 / z
			isPerspective = 0
		return (left, right, bottom, top, near, far, isPerspective)


if __name__ == "__main__":
	# unit test
	s = CRMatrix()
	s.ZRotate(40)
	s.Print()
	print ""
	s = CRMatrix()
	s.Rotate(40, 0, 0, 1)
	s.Print()

	l0 = -1.1
	r0 = 1.6
	b0 = -2.0
	t0 = 1.5
	n0 = 12.0
	f0 = 84.0
	p = CRMatrix()
	p.Ortho(l0, r0, b0, t0, n0, f0)
	(l1, r1, b1, t1, n1, f1, p) = p.DecomposeProjection()
	print "ortho in:  %f, %f, %f, %f, %f, %f" % (l0, r0, b0, t0, n0, f0)
	print "ortho out: %f, %f, %f, %f, %f, %f" % (l1, r1, b1, t1, n1, f1)

	p = CRMatrix()
	p.Frustum(l0, r0, b0, t0, n0, f0)
	(l1, r1, b1, t1, n1, f1, p) = p.DecomposeProjection()
	print "frustum in:  %f, %f, %f, %f, %f, %f" % (l0, r0, b0, t0, n0, f0)
	print "frustum out: %f, %f, %f, %f, %f, %f" % (l1, r1, b1, t1, n1, f1)


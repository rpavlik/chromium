/*
 * Utility functions for non-planar tilesort.
 * Hide some of the complexity of many calls to
 * glChromiumParametervCR_ptr(GL_SERVER_VIEW_MATRIX_CR, ...) and
 * glChromiumParametervCR_ptr(GL_SERVER_PROJECTION_MATRIX_CR, ...)
 */

#include "chromium.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "multiview.h"


static const GLfloat Identity[16] = {
   1, 0, 0, 0,
   0, 1, 0, 0,
   0, 0, 1, 0,
   0, 0, 0, 1
};


static glChromiumParametervCRProc glChromiumParametervCR_ptr = NULL;


/*
 * Set the viewing or projection matrix to the identity for all servers.
 * Input: target - either GL_SERVER_VIEW_MATRIX_CR or
 *                 GL_SERVER_PROJECTION_MATRIX_CR.
 *        numServers - number of Chromium servers
 */
void
MultiviewLoadIdentity(GLenum target, int numServers)
{
	GLfloat v[17];
	int i;

	if (!glChromiumParametervCR_ptr) {
		glChromiumParametervCR_ptr = (glChromiumParametervCRProc) GET_PROC("glChromiumParametervCR");
		if (!glChromiumParametervCR_ptr) {
			fprintf(stderr, "MultiviewLoadIdentity: Unable to get pointer to glChromiumParametervCR");
			return;
		 }
	}

	assert(target == GL_SERVER_VIEW_MATRIX_CR ||
				 target == GL_SERVER_PROJECTION_MATRIX_CR);

	memcpy(v + 1, Identity, sizeof(Identity));
	for (i = 0; i < numServers; i++) {
		v[0] = (GLfloat)i; /* the server */
#if USE_CHROMIUM
		glChromiumParametervCR_ptr(target, GL_FLOAT, 17, v);
#endif
	}
}


static void
Normalize(float *x, float *y, float *z)
{
	 float m = (float)sqrt(*x * *x + *y * *y + *z * *z);
	 *x /= m;
	 *y /= m;
	 *z /= m;
}


/*
 * Set the server's viewing and projection matrices to show the view
 * specified by the eye, direction, and up parameters.
 * Input: server - the server to update
 *        eyeX, eyeY, eyeZ - eye position within the unit cube.
 *        dirX, dirY, dirZ - viewing direction
 *        upX, upY, upZ - the up vector
 */
void
MultiviewFrustum(int server, float boxSize, float farClip,
								 float eyeX, float eyeY, float eyeZ,
								 float dirX, float dirY, float dirZ,
								 float upX, float upY, float upZ)
{
	GLfloat rightX, rightY, rightZ;
	GLfloat eX, eY, eZ;
	GLfloat frustum[7], v[17];

	if (!glChromiumParametervCR_ptr) {
		glChromiumParametervCR_ptr = (glChromiumParametervCRProc) GET_PROC("glChromiumParametervCR");
		if (!glChromiumParametervCR_ptr) {
			fprintf(stderr, "MultiviewLoadIdentity: Unable to get pointer to glChromiumParametervCR");
			return;
		 }
	}

	/* change from RH to LH coord sys */
	dirZ = -dirZ;

	/*** Compute orthonormal basis vectors ***/

	/* right(X) = up(Y) cross dir(Z) */
	rightX =  upY * dirZ - upZ * dirY;
	rightY = -upX * dirZ + upZ * dirX;
	rightZ =  upX * dirY - upY * dirX;

	/* up(Y) = dir(X) cross right(Z) */
	upX =  dirY * rightZ - dirZ * rightY;
	upY = -dirX * rightZ + dirZ * rightX;
	upZ =  dirX * rightY - dirY * rightX;

	/* dir(Z) = right(x) cross up(y) */
	dirX =  rightY * upZ - rightZ * upY;
	dirY = -rightX * upZ + rightZ * upX;
	dirZ =  rightX * upY - rightY * upX;

	Normalize(&rightX, &rightY, &rightZ);
	Normalize(&upX, &upY, &upZ);
	Normalize(&dirX, &dirY, &dirZ);

	/* transform/rotate eye position into viewing space */
	eX = eyeX * rightX + eyeY * rightY + eyeZ * rightZ;
	eY = eyeX * upX    + eyeY * upY    + eyeZ * upZ;
	eZ = eyeX * dirX   + eyeY * dirY   + eyeZ * dirZ;

	/*** Projection/frustum ***/
	frustum[0] = (float) server;
	/* Skew the view frustum according to eye position (i.e. the
	 * person's position in the cave.
	 * XX This is also where we'd specify an interocular distance for
	 * stereo.
	 */
	frustum[1] = -boxSize - eX;  /* left */
	frustum[2] =  boxSize - eX;  /* right */
	frustum[3] = -boxSize - eY;  /* bottom */
	frustum[4] =  boxSize - eY;  /* top */
	frustum[5] =  boxSize - eZ;  /* near */
	frustum[6] = farClip;      /* far */
	glChromiumParametervCR_ptr(GL_SERVER_FRUSTUM_CR, GL_FLOAT, 7, frustum);
#if 0
	printf("New Frustum %d: %f, %f, %f, %f, %f, %f\n", server,
				 frustum[1], frustum[2], frustum[3], frustum[4],
				 frustum[5], frustum[6]);
#endif

	/*** View matrix ***/
	v[0] = (float) server;

	v[1] = rightX;
	v[2] = rightY;
	v[3] = rightZ;
	v[4] = 0;

	v[5] = upX;
	v[6] = upY;
	v[7] = upZ;
	v[8] = 0;

	v[9] = dirX;
	v[10] = dirY;
	v[11] = dirZ;
	v[12] = 0;

	v[13] = 0;
	v[14] = 0;
	v[15] = 0;
	v[16] = 1;

	glChromiumParametervCR_ptr(GL_SERVER_VIEW_MATRIX_CR, GL_FLOAT, 17, v);
#if 0
	{
		printf("view matrix:\n");
		int i;
		for (i = 1; i < 17; i++) {
			printf("%f ", v[i]);
			if (i % 4 == 0)
				printf("\n");
		}
		printf("\n");
	}
#endif
}


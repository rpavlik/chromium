/*
 * Utility functions for non-planar tilesort.
 * Hide some of the complexity of many calls to
 * glChromiumParametervCR_ptr(GL_SERVER_VIEW_MATRIX_CR, ...) and
 * glChromiumParametervCR_ptr(GL_SERVER_PROJECTION_MATRIX_CR, ...)
 */


#ifndef MULTIVIEW_H
#define MULTIVIEW_H


extern void
MultiviewLoadIdentity(GLenum target, int numServers);


extern void
MultiviewFrustum(int server, float boxSize, float farClip,
								 float eyeX, float eyeY, float eyeZ,
								 float dirX, float dirY, float dirZ,
								 float upX, float upY, float upZ);

#endif

/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef BINARYSWAP_SPU_H
#define BINARYSWAP_SPU_H

#ifdef WINDOWS
#define BINARYSWAPSPU_APIENTRY __stdcall
#else
#define BINARYSWAPSPU_APIENTRY
#endif

#include "cr_hash.h"
#include "cr_spu.h"
#include "cr_server.h"
#include "cr_threads.h"

#define BINARYSWAP_SPU_PORT	8192
#define CLEAR_BARRIER           52001
#define SWAP_BARRIER            52002
#define POST_SWAP_BARRIER       52003
#define CREATE_CONTEXT_BARRIER  52004
#define MAKE_CURRENT_BARRIER    52005
#define DESTROY_CONTEXT_BARRIER 52006
#define MUTEX_SEMAPHORE         52007

typedef struct {
	GLint index;         /**< my window number */
	GLint renderWindow;  /**< the super (render SPU) window */
	GLint childWindow;   /**< the child SPU's window handle */
	GLint width, height;
	GLint childWidth, childHeight;
	GLubyte *msgBuffer;
	GLint bytesPerColor, bytesPerDepth;  /**< bytes per pixel */
	GLenum depthType;  /**< GL_UNSIGNED_SHORT or GL_FLOAT */
	
	int* read_x;
	int* read_y;
	int* read_width;
	int* read_height;

	/**< window-space union of all bounding boxes */
	CRrecti bboxUnion;
} WindowInfo;

/** Message header */
typedef struct {
	CRMessageHeader header;
	float depth;
	int start_x, start_y;
	int width, height;
	int clipped_x, clipped_y;
	int clipped_width, clipped_height;
} BinarySwapMsg;

typedef struct {
	GLboolean inUse;
	GLint renderContext;
	GLint childContext;
	WindowInfo *currentWindow;
} ContextInfo;

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;
	CRServer *server;

	/** config options */
	/*@{*/
	int resizable;
	int local_visualization;
	int renderToAppWindow;
	/*@}*/

	CRHashTable *contextTable;
	CRHashTable *windowTable;

#ifndef CHROMIUM_THREADSAFE
	ContextInfo *currentContext;
#endif

	GLint barrierSize;

	/**< Store a list of all nodes in our swap network */
	char ** peer_names;
	
	/**< Stor a list of all nodes we will be swapping with */
	char ** swap_partners;
		
	/**< What is our number in the network. Used to find swap partners
	   from network list */
	int node_num;
	
	/**< What type of compositing will we be doing? */
	int alpha_composite;
	int depth_composite;
	
	/**< Are we attempting to clip window? */
	int clipped_window;
	
	/**< How many peers do we have? */
	int num_peers;
	
	/**< What MTU do we use? */
	unsigned int mtu;
	
	/**< Used to store connections for swapping */
	CRConnection **peer_send, **peer_recv;
		
	/**< The offset for the header of the message */
	int offset;
	
	/**< How many times do we swap? */
	int stages;
	
	/**< Store if we are the top/bottom or left/right of swap */
	int *highlow;
	
	/**< What is the depth of the frame buffer */
	float depth;
	
} Binaryswapspu;

extern Binaryswapspu binaryswap_spu;

#ifdef CHROMIUM_THREADSAFE
extern CRtsd _BinaryswapTSD;
#define GET_CONTEXT(T)  ContextInfo *T = crGetTSD(&_BinaryswapTSD)
#else
#define GET_CONTEXT(T)  ContextInfo *T = binaryswap_spu.currentContext
#endif


extern void binaryswapspuGatherConfiguration( Binaryswapspu *spu );

#endif /* BINARYSWAP_SPU_H */

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

#include "cr_spu.h"
#include "cr_server.h"
#include "cr_threads.h"
#include "cr_net.h"

#define MAX_WINDOWS             32
#define MAX_CONTEXTS            32
#define BINARYSWAP_SPU_PORT     8192
#define BINARYSWAP_BARRIER      1
#define CREATE_CONTEXT_BARRIER  2
#define MAKE_CURRENT_BARRIER    3
#define DESTROY_CONTEXT_BARRIER 4


typedef struct {
  unsigned long id;
  int currentContext;
  int currentWindow;
#ifndef WINDOWS
  Display *dpy;
#endif
} ThreadInfo;

/* Message header */
typedef struct {
  CRMessageHeader header;
  float depth;
  int start_x, start_y;
  int width, height;
} BinarySwapMsg;


typedef struct {
  GLboolean inUse;
  GLint renderWindow;
  GLint childWindow;
  GLint width, height;
  GLubyte *colorBuffer;
  GLvoid *depthBuffer;
  GLenum depthType;  /* GL_UNSIGNED_SHORT or GL_FLOAT */
} WindowInfo;

typedef struct {
  GLboolean inUse;
  GLint renderContext;
  GLint childContext;
  CRContext *tracker;  /* for tracking matrix state */
} ContextInfo;

typedef struct {
  int id;
  int has_child;
  SPUDispatchTable self, child, super;
  CRServer *server;
  
  int extract_depth;
  int extract_alpha;
  int local_visualization;
  int visualize_depth;
  int drawX, drawY;
  int resizable;
  
  WindowInfo windows[MAX_WINDOWS];
  
  ContextInfo contexts[MAX_CONTEXTS];
  
#ifndef CHROMIUM_THREADSAFE
  ThreadInfo singleThread;
#endif
  
  GLint renderWindow;
  GLint renderContext;
  GLint childWindow;
  GLint childContext;
  
  GLint barrierCount;
  
  float halfViewportWidth, halfViewportHeight, viewportCenterX, viewportCenterY;
  int cleared_this_frame;

  /* Store a list of all nodes in our swap network */
  char ** peer_names;
  
  /* Stor a list of all nodes we will be swapping with */
  char ** swap_partners;

  /* How many nodes do we have? */
  int numnodes;

  /* What is our number in the network. Used to find swap partners
     from network list */
  int node_num;

  /* What type of compositing will we be doing? */
  int alpha_composite;
  int depth_composite;

  /* How many peers do we have? */
  int num_peers;

  /* What MTU do we use? */
  unsigned int mtu;

  /* Used to store connections for swapping */
  CRConnection **peer_send, **peer_recv;

  /* The message to send out */
  GLubyte* outgoing_msg;

  /* The offset for the header of the message */
  int offset;

  /* How many times do we swap? */
  int stages;

  /* Store if we are the top/bottom or left/right of swap */
  int *highlow;

  /* What is the depth of the frame buffer */
  float depth;

  /* Stores the bounding box if used */
  struct { float xmin, ymin, zmin, xmax, ymax, zmax; } *bbox;

} BinaryswapSPU;

extern BinaryswapSPU binaryswap_spu;

#ifdef CHROMIUM_THREADSAFE
extern CRtsd _BinaryswapTSD;
#define GET_THREAD(T)  ThreadInfo *T = crGetTSD(&_BinaryswapTSD)
#else
#define GET_THREAD(T)  ThreadInfo *T = &(binaryswap_spu.singleThread)
#endif


extern void binaryswapspuGatherConfiguration( BinaryswapSPU *spu );

#endif /* BINARYSWAP_SPU_H */

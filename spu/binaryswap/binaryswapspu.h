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
#include "cr_net.h"

#define BINARYSWAP_SPU_PORT 8192
#define BSWAP_BARRIER 1

/* Message header */
typedef struct {
  CRMessageHeader header;
  float depth;
  int start_x, start_y;
  int width, height;
} BinarySwapMsg;

/* Bounding box layout */
typedef struct {
  float x1, y1, z1, x2, y2, z2;
} BBox;

void binaryswapspuGatherConfiguration( void );

typedef struct {
  int id;
  int has_child;
  SPUDispatchTable self, child, super;
  
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
  int mtu;

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
  BBox* bounding_box;

} BinaryswapSPU;

extern BinaryswapSPU binaryswap_spu;

#endif /* BINARYSWAP_SPU_H */









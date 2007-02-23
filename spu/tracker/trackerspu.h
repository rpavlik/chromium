/* 
 * Copyright (c) 2006  Michael Duerig  
 * Bern University of Applied Sciences
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */

/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef TRACKER_SPU_H
#define TRACKER_SPU_H

#ifndef WINDOWS
#  include <sys/socket.h>
#endif

#include "cr_spu.h"
#include "cr_server.h"
#include "trackerspu_udp.h"
#include "cr_matrix.h"
#include "cr_threads.h"

#ifdef WINDOWS
#  define TRACKERSPU_APIENTRY __stdcall
#  define snprintf _snprintf
  typedef SOCKET crSocket;
#else
#  define TRACKERSPU_APIENTRY 
  typedef int crSocket;
#endif


typedef enum {LEFT = 0, RIGHT = 1} Eyes;

typedef struct {
  CRmatrix orientation;     // Orientation of this screen
  CRmatrix screenMatrix;    // Transformation from world coordinates to local screen coordinates
  GLfloat width, height;    // The width and height of the screen
  GLfloat znear, zfar;      // Position of near and far clipping plane for this screen
  GLvectorf Offset;         // Offset applied to tracker data for this screen
  Eyes eye;                 // For stereo mode: indicates to which eye this screen belongs 
  int serverIndex;          // Index of the crServer instance to which this screeen belongs
} crScreen;

typedef struct {            
  GLvectorf left;           // Position of left eye as received from the tracker
  GLvectorf right;          // Position of right eye as received from the tracker
  int dirty;                // Indicates fresh data. Set by socket thread, cleared by main thread 
} TrackerPos;

/**
 * Tracker SPU descriptor
 */
typedef struct {
	int id; /**< Spu id */
	int has_child; 	/**< Spu has a child  Not used */
	SPUDispatchTable self, child, super;	/**< SPUDispatchTable self for this SPU, child spu and super spu */
	CRServer *server;	/**< crserver descriptor */

  char *listenIP;           // Internet address on which to listen for incoming connections
  int listenPort;           // Port on which to listen for incoming connections
  CRmatrix viewMatrix;      // Users view matrix
  crSocket listen_sock;     // Socket handle of listening socket for reception of tracker data
  CRthread socketThread;    // Socket thread 

  TrackerPos pos[3];        // Receive buffers for communication with the tracker
  TrackerPos *nextPos;      // The socket thread writes to nextPos and then atomically exchanges nextPos with freePos
  TrackerPos *currentPos;   // The main thread reads from then atomically exchanges currentPos with freePos
  TrackerPos *freePos;      

  CRmatrix caveMatrix;      // Transformation from tracker to world coordinates
  CRmatrix leftEyeMatrix;   // Transformation from sensor to left eye coordinates
  CRmatrix rightEyeMatrix;  // Transformation from sensor to right eye coordinates
  int screenCount;          // Number of screens for which we have to maintain view and projection information
  crScreen* screens;        // The screens
} TrackerSPU;

/** Tracker state descriptor */
extern TrackerSPU tracker_spu;

/** Named SPU functions */
extern SPUNamedFunctionTable _cr_tracker_table[];

/** Option table for SPU */
extern SPUOptions trackerSPUOptions[];

extern void trackerspuGatherConfiguration( void );


#endif /* TRACKER_SPU_H */

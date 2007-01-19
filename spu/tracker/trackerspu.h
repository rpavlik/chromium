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
#  include <pthread.h>
#endif

#include "cr_spu.h"
#include "cr_server.h"
#include "trackerspu_udp.h"
#include "cr_matrix.h"

#ifdef WINDOWS
#  define TRACKERSPU_APIENTRY __stdcall
#  define snprintf _snprintf
  typedef SOCKET crSocket;
  typedef HANDLE crThread;
#else
#  define TRACKERSPU_APIENTRY 
  typedef int crSocket;
  typedef pthread_t crThread;
#endif


typedef enum {LEFT = 0, RIGHT = 1} Eyes;

typedef struct {
  CRmatrix orientation;     // Orientation of this screen
  CRmatrix openGL2Screen;   // Transformation from OpenGL coordinates to local screen coordinates
  GLfloat width, height;    // The width and height of the screen
  GLfloat znear, zfar;      // Position of near and far clipping plane for this screen
  GLvectorf Offset;         // Offset applied to tracker data for this screen
  Eyes eye;                 // For stereo mode: indicates to which eye this screen belongs 
  int serverIndex;          // Index of the crServer instance to which this screeen belongs
} crScreen;

typedef struct {            
  GLvectorf left;            // Position of left eye as received from the tracker
  GLvectorf right;           // Position of right eye as received from the tracker
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
  crSocket listen_sock;     // Socket handle of listening socket for reception of tracker data
  crThread hSocketThread;   // Handle of socket thread 
  TrackerPos pos[2];        // Receive buffers for communication with the tracker
  int currentIndex;         // Index to the current receive buffer
  TrackerPos *currentPos;   // Current tracker position
  int hasNewPos;            // Indicates that new tracker data is available (i.e. currentPos has changed)
  CRmatrix tracker2OpenGL;  // Transformation from Tracker coordinates to OpenGL coordinates
  GLvectorf rightEyeOffset; // Multiplying rightEyeOffset with the tracker's rotation matrix yields left - right 
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

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

#ifndef TRACKERSPU_UDP_H
#define TRACKERSPU_UDP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cr_matrix.h"

  
typedef struct {
  GLvectorf left;
  CRmatrix rot;
} TrackerPose;

/**
 * Start UDP server which listens for changes of the pose sent from the tracker
 */
void trackerspuStartUDPServer(void);

/**
 * Stop the UDP server
 */
void trackerspuStopUDPServer(void);

/**
 * Pack the tracker pose into the buffer buf for sending over the network. Returns the number of 
 * bytes copied into buf or 0 for failure (i.e. when the supplied buffer is too small).
 */
int packTrackerPose(const TrackerPose* pose, void *buf, int len);

/**
 * Unpack a tracker pose from a buffer buf received over the network. Returns TRUE on 
 * success, FALSE otherwise (i.e. when the buffer contains garbage).
 */
int unpackTrackerPose(TrackerPose *pose, const void* buf);

#ifdef __cplusplus
}
#endif

#endif 

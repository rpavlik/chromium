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
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
        
#include "trackerspu.h"

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_mem.h"

/**
 * Set default options for SPU
 */
static void setDefaults( void )
{
}

static void debugPrintMatrix(const char *msg, const CRmatrix *m) {
  crDebug(
    "%s: \n" 
    "  %f %f %f %f\n"
    "  %f %f %f %f\n"
   	"  %f %f %f %f\n"
    "  %f %f %f %f\n",
    msg,
    m->m00, m->m10, m->m20, m->m30,
    m->m01, m->m11, m->m21, m->m31,
    m->m02, m->m12, m->m22, m->m32,
    m->m03, m->m13, m->m23, m->m33);
}

static void setListenIP(void *spu, const char *response) {
  TrackerSPU *s = (TrackerSPU*)spu;
  crFree(s->listenIP);
  s->listenIP = crStrdup(response);
  crDebug("Tracker SPU: listen_ip = %s", s->listenIP);
}

static void setListenPort(void *spu, const char *response) {
  TrackerSPU *s = (TrackerSPU*)spu;
  sscanf(response, "%u", &(s->listenPort)); 
  crDebug("Tracker SPU: listen_port = %u", s->listenPort);
}

static void setScreenCount(void *spu, const char *response) {
  TrackerSPU *s = (TrackerSPU*)spu;
  sscanf(response, "%u", &(s->screenCount));
  crDebug("Tracker SPU: screen_count = %u", s->screenCount);
}

static void setTrackerMatrix(void *spu, const char *response) {
  TrackerSPU *s = (TrackerSPU*)spu;
  crMatrixInitFromString(&(s->caveMatrix), response);
  debugPrintMatrix("Tracker SPU: cave_matrix = ", &tracker_spu.caveMatrix);
}

static void setLeftEyeMatrix(void *spu, const char *response) {
  TrackerSPU *s = (TrackerSPU*)spu;
  crMatrixInitFromString(&(s->leftEyeMatrix), response);
  debugPrintMatrix("Tracker SPU: left_eye_matrix = ", &tracker_spu.leftEyeMatrix); 
}

static void setRightEyeMatrix(void *spu, const char *response) {
  TrackerSPU *s = (TrackerSPU*)spu;
  crMatrixInitFromString(&(s->rightEyeMatrix), response);
  debugPrintMatrix("Tracker SPU: right_eye_matrix = ", &tracker_spu.rightEyeMatrix); 
}

// Note: This option depends on right_eye_offset
static void setInitialPos(void *spu, const char *response) {
  TrackerSPU *s = (TrackerSPU*)spu;
	const char *fmt = "%f, %f, %f";
	const char *fmtb = "[ %f, %f, %f]";

  s->currentPos = &(s->pos[0]);
  s->nextPos = &(s->pos[1]);
  s->freePos = &(s->pos[2]);
  
	if (sscanf(response, (response[0] == '[' ? fmtb : fmt),
        &(s->freePos->left.x),
        &(s->freePos->left.y),
        &(s->freePos->left.z)) != 3) 
  {
    crWarning("Tracker SPU: Invalid value '%s' for initial_pos. Defaulting to '[0, 0, 0]'", response);
    s->freePos->left.x = 0;
    s->freePos->left.y = 0;
    s->freePos->left.z = 0;
  }

  s->freePos->right.x = s->freePos->left.x;
  s->freePos->right.y = s->freePos->left.y;
  s->freePos->right.z = s->freePos->left.z;

  s->freePos->left.w = 1;
  s->freePos->right.w = 1;
  s->freePos->dirty = 1;

  crDebug("Tracker SPU: initial_pos = [%f, %f, %f]", 
    s->freePos->left.x, s->freePos->left.y, s->freePos->left.z);
}

static void setViewMatrix(void *spu, const char *response) {
  TrackerSPU *s = (TrackerSPU*)spu;
  crMatrixInitFromString(&(s->viewMatrix), response);
  debugPrintMatrix("Tracker SPU: vieMatrix = ", &tracker_spu.viewMatrix); 
}

static void setScreenServerIndex(crScreen *scr, const char *response, int index) {
  if (sscanf(response, "%u", &(scr->serverIndex)) != 1)
    crError("Tracker SPU: Invalid value '%s' for screen_%d_server_index.", response, index);

  crDebug("Tracker SPU: screen_%d_server_index = %u", index, scr->serverIndex);
}

static void setScreenOrientation(crScreen *scr, const char *response, int index) {
  float a, x, y, z;
	const char *fmt = "%f, %f, %f, %f";
	const char *fmtb = "[ %f, %f, %f, %f ]";

	if (sscanf(response, (response[0] == '[' ? fmtb : fmt), &a, &x, &y, &z) != 4)
    crError("Tracker SPU: Invalid value '%s' for screen_%d_orientation.", response, index);
  
  // Calculate rotational part of view matrix 
  crMatrixInit(&scr->orientation);
  crMatrixRotate(&scr->orientation, -a, x, y, z);
  crDebug("Tracker SPU: screen_%d_orientation = ", index);
  debugPrintMatrix("", &scr->orientation); 
}

// Note: This option depends on screen_%d_orientation
static void setScreenOrigin(crScreen *scr, const char *response, int index) {
  float x, y, z;
	const char *fmt = "%f, %f, %f";
	const char *fmtb = "[ %f, %f, %f ]";

	if (sscanf(response, (response[0] == '[' ? fmtb : fmt), &x, &y, &z) != 3)
    crError("Tracker SPU: Invalid value '%s' for screen_%d_origin.", response, index);

  // Calculate screen transformation
  scr->screenMatrix = scr->orientation;
  crMatrixTranslate(&scr->screenMatrix, -x, -y, -z);
  crDebug("Tracker SPU: screen_%d_origin = ", index);
  debugPrintMatrix("", &scr->screenMatrix); 
}

static void setScreenExtent(crScreen *scr, const char *response, int index) {
	const char *fmt = "%f, %f, %f, %f";
	const char *fmtb = "[ %f, %f, %f, %f ]";

	if (sscanf(response, (response[0] == '[' ? fmtb : fmt), &scr->width, &scr->height, &scr->znear, &scr->zfar) != 4)
    crError("Tracker SPU: Invalid value '%s' for screen_%d_extend.", response, index);

  crDebug("Tracker SPU screen_%d_extend [width, height, near, far] = [%f, %f, %f, %f]", 
    index, scr->width, scr->height, scr->znear, scr->zfar);
}

static void setScreenEye(crScreen *scr, const char *response, int index) {
  if (crStrcasecmp(response, "left") == 0) {
    scr->eye = LEFT;
    crDebug("Tracker SPU: screen_%d_eye = LEFT", index);
  }
  else if (crStrcasecmp(response, "right") == 0) {
    scr->eye = RIGHT;
    crDebug("Tracker SPU: screen_%d_eye = RIGHT", index);
  }
  else {
    crWarning("Tracker SPU: Invalid value '%s' for screen_%d_eye. Defaultig to 'LEFT'.", response, index);
    scr->eye = LEFT;
  }
}

static void setScreenOffset(crScreen *scr, const char *response, int index) {
	const char *fmt = "%f, %f, %f";
	const char *fmtb = "[ %f, %f, %f ]";

	if (sscanf(response, (response[0] == '[' ? fmtb : fmt), &scr->Offset.x, &scr->Offset.y, &scr->Offset.z) != 3) {
    crWarning("Tracker SPU: Invalid value '%s' for screen_%d_offset. Defaultig to '[0, 0, 0]'.", response, index);
    scr->Offset.x = 0;
    scr->Offset.y = 0;
    scr->Offset.z = 0;
  }

  crDebug("Tracker SPU: screen_%d_offset = [%f, %f, %f]", 
    index, scr->Offset.x, scr->Offset.x, scr->Offset.x);
}

/** 
 * SPU options
 * option, type, nr, default, min, max, title, callback
 */
// Note: initial_pos depends on right_eye_offset. The former must come first thus.
SPUOptions trackerSPUOptions[] = {
   { "listen_ip", CR_STRING, 1, NULL, NULL, NULL, "Listen IP", (SPUOptionCB)setListenIP },
   { "listen_port", CR_INT, 1, "1234", NULL, NULL, "Listen port", (SPUOptionCB)setListenPort },
   { "screen_count", CR_INT, 1, "1", NULL, NULL, "Screen count", (SPUOptionCB)setScreenCount },
   { "cave_matrix", CR_STRING, 1, "[1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]", NULL, NULL, "Transformation from tracker to world coordinates", (SPUOptionCB)setTrackerMatrix },
   { "left_eye_matrix", CR_STRING, 1, "[1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]", NULL, NULL, "Transformation from sensor to left eye", (SPUOptionCB)setLeftEyeMatrix },
   { "right_eye_matrix",CR_STRING, 1, "[1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]", NULL, NULL, "Transformation from sensor to right eye", (SPUOptionCB)setRightEyeMatrix },
   { "initial_pos", CR_STRING, 1, "[0, 0, 0]", NULL, NULL, "Initial position", (SPUOptionCB)setInitialPos },
   { "view_matrix", CR_STRING, 1, "[1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]", NULL, NULL, "User view matrix", (SPUOptionCB)setViewMatrix },
   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


static void readScreenParams(CRConnection *conn, int index, crScreen *scr) {
  enum { OPT_SIZE = 64 };
  enum { RESPONSE_SIZE = 8096 };

  SPUOptions su;
  char opt[OPT_SIZE];
  char response[RESPONSE_SIZE];

  memset(&su, 0, sizeof(su));

  snprintf(opt, OPT_SIZE, "screen_%d_server_index", index);
	su.option = opt;
	su.type = CR_INT;
  su.numValues = 1;

  if (crMothershipGetSPUParam(conn, response, su.option)) 
    setScreenServerIndex(scr, response, index);
  else
    crError("Tracker SPU: Failed to read option %s", su.option);


  snprintf(opt, OPT_SIZE, "screen_%d_orientation", index);
	su.option = opt;
	su.type = CR_STRING;
  su.numValues = 1;

  if (crMothershipGetSPUParam(conn, response, su.option)) 
    setScreenOrientation(scr, response, index);
  else
    crError("Tracker SPU: Failed to read option %s", su.option);
  

  // Make sure option screen_%d_orientation has already been read when reading this one!
  snprintf(opt, OPT_SIZE, "screen_%d_origin", index);
	su.option = opt;
	su.type = CR_STRING;
  su.numValues = 1;

  if (crMothershipGetSPUParam(conn, response, su.option)) 
    setScreenOrigin(scr, response, index);
  else
    crError("Tracker SPU: Failed to read option %s", su.option);


  snprintf(opt, OPT_SIZE, "screen_%d_extent", index);
	su.option = opt;
	su.type = CR_STRING;
  su.numValues = 1;

  if (crMothershipGetSPUParam(conn, response, su.option)) 
    setScreenExtent(scr, response, index);
  else
    crError("Tracker SPU: Failed to read option %s", su.option);

  snprintf(opt, OPT_SIZE, "screen_%d_offset", index);
  su.option = opt;
  su.type = CR_STRING;
  su.numValues = 1;

  response[0] = 0;
  crMothershipGetSPUParam(conn, response, su.option);
  setScreenOffset(scr, response, index);

  snprintf(opt, OPT_SIZE, "screen_%d_eye", index);
  su.option = opt;
  su.type = CR_STRING;
  su.numValues = 1;

  response[0] = 0;
  crMothershipGetSPUParam(conn, response, su.option);
  setScreenEye(scr, response, index);
}

/**
 * Gather the config info for the SPU
 */
void trackerspuGatherConfiguration( void )
{
	CRConnection *conn;
  int i;

	setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
		crSPUSetDefaultParams( &tracker_spu, trackerSPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, tracker_spu.id );

	crSPUGetMothershipParams( conn, &tracker_spu, trackerSPUOptions );

  tracker_spu.screens = (crScreen *)crAlloc(tracker_spu.screenCount*sizeof(crScreen));
  for (i = 0; i < tracker_spu.screenCount; i++)
    readScreenParams(conn, i, &(tracker_spu.screens[i]));

	crMothershipDisconnect( conn );
}

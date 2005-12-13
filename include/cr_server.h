/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef INCLUDE_CR_SERVER_H
#define INCLUDE_CR_SERVER_H

#include "cr_spu.h"
#include "cr_net.h"
#include "cr_hash.h"
#include "cr_protocol.h"
#include "cr_glstate.h"
#include "spu_dispatch_table.h"

#include "state/cr_currentpointers.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CR_MAX_WINDOWS 100
#define CR_MAX_CLIENTS 20

typedef struct {
	CRrecti imagewindow;    /**< coordinates in mural space */
	CRrectf bounds;         /**< normalized coordinates in [-1,-1] x [1,1] */
	CRrecti outputwindow;   /**< coordinates in server's rendering window */
	CRrecti clippedImagewindow;  /**< imagewindow clipped to current viewport */
	CRmatrix baseProjection;  /**< pre-multiplied onto projection matrix */
	CRrecti scissorBox;     /**< passed to back-end OpenGL */
	CRrecti viewport;       /**< passed to back-end OpenGL */
	GLuint serialNo;        /**< an optimization */
} CRExtent;

struct BucketingInfo;

/**
 * Mural info
 */
typedef struct {
	int width, height;
	CRrecti imagespace;                /**< the whole mural rectangle */
	int curExtent;
	int numExtents;                    /**< number of tiles */
	CRExtent extents[CR_MAX_EXTENTS];  /**< per-tile info */
	int maxTileHeight;                 /**< the tallest tile's height */

	/** optimized, hash-based tile bucketing */
	int optimizeBucket;
	struct BucketingInfo *bucketInfo;

	unsigned int underlyingDisplay[4]; /**< needed for laying out the extents */

	GLboolean viewportValidated;

	int spuWindow;                     /**< the SPU's corresponding window ID */
} CRMuralInfo;

/**
 * A client is basically an upstream Cr Node (connected via mothership)
 */
typedef struct _crclient {
	int spu_id;        /**< id of the last SPU in the client's SPU chain */
	CRConnection *conn;       /**< network connection from the client */
	int number;        /**< a unique number for each client */
	GLint currentContextNumber;
	CRContext *currentCtx;
	GLint currentWindow;
	CRMuralInfo *currentMural;
	GLint windowList[CR_MAX_WINDOWS];
} CRClient;


typedef struct CRPoly_t {
	int npoints;
	double *points;
	struct CRPoly_t *next;
} CRPoly;

/**
 * There's one of these run queue entries per client
 * The run queue is a circular, doubly-linked list of these objects.
 */
typedef struct RunQueue_t {
	CRClient *client;
	int blocked;
	struct RunQueue_t *next;
	struct RunQueue_t *prev;
} RunQueue;

typedef struct {
	unsigned short tcpip_port;

	int numClients;
	CRClient *clients[CR_MAX_CLIENTS];  /**< array [numClients] */
	CRClient *curClient;
	CRCurrentStatePointers current;

	GLboolean firstCallCreateContext;
	GLboolean firstCallMakeCurrent;
	GLint currentWindow;
	GLint currentNativeWindow;

	CRHashTable *muralTable;  /**< hash table where all murals are stored */

	int client_spu_id;

	int mtu;
	int buffer_size;
	char protocol[1024];

	SPU *head_spu;
	SPUDispatchTable dispatch;

	CRNetworkPointer return_ptr;
	CRNetworkPointer writeback_ptr;

	CRLimitsState limits; /**< GL limits for any contexts we create */

	int SpuContext; /**< Rendering context for the head SPU */
	int SpuContextVisBits; /**< Context's visual attributes */

	CRHashTable *contextTable;  /**< hash table for rendering contexts */
	CRContext *DummyContext;    /**< used when no other bound context */

	CRHashTable *programTable;  /**< for vertex programs */
	GLuint currentProgram;

	/** configuration options */
	/*@{*/
	int useL2;
	int ignore_papi;
	unsigned int maxBarrierCount;
	unsigned int clearCount;
	int optimizeBucket;
	int only_swap_once;
	int debug_barriers;
	int sharedDisplayLists;
	int sharedTextureObjects;
	int sharedPrograms;
	int sharedWindows;
	int localTileSpec;
	int useDMX;
	int overlapBlending;
	int vpProjectionMatrixParameter;
	const char *vpProjectionMatrixVariable;
	int stereoView;
	int vncMode;   /* cmd line option */
	/*@}*/
	/** view_matrix config */
	/*@{*/
	GLboolean viewOverride;
	CRmatrix viewMatrix[2];  /**< left and right eye */
	/*@}*/
	/** projection_matrix config */
	/*@{*/
	GLboolean projectionOverride;
	CRmatrix projectionMatrix[2];  /**< left and right eye */
	int currentEye;
	/*@}*/

	/** for warped tiles */
	/*@{*/
	GLfloat alignment_matrix[16], unnormalized_alignment_matrix[16];
	/*@}*/
	
	/** tile overlap/blending info - this should probably be per-mural */
	/*@{*/
	CRPoly **overlap_geom;
	CRPoly *overlap_knockout;
	float *overlap_intens;
	int num_overlap_intens;
	int num_overlap_levels;
	/*@}*/

	CRHashTable *barriers, *semaphores;

	RunQueue *run_queue;

	GLuint currentSerialNo;
} CRServer;


extern void crServerInit( int argc, char *argv[] );
extern int CRServerMain( int argc, char *argv[] );
extern void crServerServiceClients(void);
extern void crServerAddNewClient(void);
extern SPU* crServerHeadSPU(void);
extern void crServerSetPort(int port);


#ifdef __cplusplus
}
#endif

#endif


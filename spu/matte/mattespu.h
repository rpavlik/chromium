/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef MATTE_SPU_H
#define MATTE_SPU_H

#ifdef WINDOWS
#define MATTESPU_APIENTRY __stdcall
#else
#define MATTESPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_server.h"

/* All information that has to be saved for each context. */
typedef struct {
    /* The application's actual clear color. */
    struct {
	GLclampf red, green, blue, alpha;
    } actualClearColor;

    /* How many times a glClear has been executed.
     * We use this to detect when to initialize.
     */
    unsigned int clearCount;
} ContextInfo;

// This allows us to store the current context, even in a
// threadsafe environment.
#ifdef CHROMIUM_THREADSAFE
extern CRtsd matteTSD;
#define SET_CONTEXT_INFO(C) crSetTSD(&matteTSD, C)
#define GET_CONTEXT_INFO(C) ContextInfo *C = (ContextInfo *)crGetTSD(&matteTSD)
#else
extern ContextInfo *matteCurrentContext;
#define SET_CONTEXT_INFO(C) matteCurrentContext = C
#define GET_CONTEXT_INFO(C) ContextInfo *C = matteCurrentContext
#endif


/**
 * Matte SPU descriptor
 */
typedef struct {
    int id; /**< Spu id */
    int has_child; 	/**< Spu has a child  Not used */
    SPUDispatchTable self, child, super;	/**< SPUDispatchTable self for this SPU, child spu and super spu */
    CRServer *server;	/**< crserver descriptor */

    /* A list of contexts using this SPU */
    CRHashTable *contextTable;

    /* Configuration values */
    struct {
	GLint x, y;
	GLsizei width, height;
    } matteRegion;

    /* This controls when the matte is refreshed.  If the matte is for a
     * fullscreen borderless window that will never be exposed, doing the matte
     * once at the first glClear is fine.  If the matte may require redrawing,
     * the user should configure the SPU to refresh the matte at every clear.
     */
    unsigned int matteEveryClear;

    /* Whether to use a matte color to refresh the matte.  This is the
     * only matte currently available; but an image or texture matte
     * may be available later.
     */
    unsigned int useMatteColor;
    struct {
	GLclampf red, green, blue, alpha;
    } matteColor;
} MatteSPU;

/* These are available constants for whenToMatte. */
#define MATTE_FIRST_CLEAR	0x01
#define MATTE_EVERY_CLEAR	0x02

/** Matte state descriptor */
extern MatteSPU matte_spu;

/** Named SPU functions */
extern SPUNamedFunctionTable _cr_matte_table[];

/** Option table for SPU */
extern SPUOptions matteSPUOptions[];

extern void mattespuGatherConfiguration( void );
extern void matteFreeContextInfo(void *data);

#endif /* MATTE_SPU_H */

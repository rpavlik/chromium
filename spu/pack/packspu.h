#ifndef CR_PACKSPU_H
#define CR_PACKSPU_H

#ifdef WINDOWS
#define PACKSPU_APIENTRY __stdcall
#else
#define PACKSPU_APIENTRY
#endif

#include "cr_glstate.h"
#include "cr_net.h"
#include "cr_pack.h"

void packspuCreateFunctions( void );
void packspuGatherConfiguration( void );
void packspuConnectToServer( void );

typedef struct {
	int id;

	char *server_name;
	int buffer_size;
	CRConnection *conn;

	CRPackBuffer buffer;

	CRContext *ctx;
} PackSPU;

extern PackSPU pack_spu;

#endif /* CR_PACKSPU_H */

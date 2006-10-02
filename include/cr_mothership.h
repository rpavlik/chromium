/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_MOTHERSHIP_H
#define CR_MOTHERSHIP_H

#include "cr_net.h"

#ifdef __cplusplus
extern "C" {
#endif

/* General functions */
CRConnection *crMothershipConnect( void );
int crMothershipSendString( CRConnection *conn, char *response_buf, const char *str, ... );
int crMothershipReadResponse( CRConnection *conn, void *buf );
void crMothershipDisconnect( CRConnection *conn );

/* Identification functions */
void crMothershipIdentifySPU( CRConnection *conn, int spu );
void crMothershipIdentifyOpenGL( CRConnection *conn, char *response, const char *app_id );
void crMothershipIdentify( const char *type, const char *hostname, CRConnection *conn, char *response);
void crMothershipIdentifyFaker( CRConnection *conn, char *response );
void crMothershipIdentifyServer( CRConnection *conn, char *response );
void crMothershipIdentifyCRUTServer( CRConnection *conn, char *response );
void crMothershipIdentifyCRUTClient( CRConnection *conn, char *response );
void crMothershipIdentifyCRUTProxy( CRConnection *conn, char *response );

/* Mothership functions */
void crMothershipExit( CRConnection *conn );
void crMothershipReset( CRConnection *conn );
void crMothershipSetParam( CRConnection *conn, const char *param, const char *value );
int crMothershipGetParam( CRConnection *conn, const char *param, char *value );
int crMothershipGetMTU( CRConnection *conn );

/* SPU functions */
void crMothershipGetServers( CRConnection *conn, char *response );
int crMothershipGetSPUParam( CRConnection *conn, char *response, const char *param);
int crMothershipGetServerParamFromSPU( CRConnection *conn, int server_num, const char *param, char *response );
int crMothershipGetTiles( CRConnection *conn, char *response, int server_num );
int crMothershipGetDisplays( CRConnection *conn, char *response );
int crMothershipGetDisplayTiles( CRConnection *conn, char *response, int server_num );
int crMothershipGetSPURank( CRConnection *conn );


/* Server node functions */
void crMothershipGetClients( CRConnection *conn, char *response );
int crMothershipGetServerParam( CRConnection *conn, char *response, const char *param);
int crMothershipGetServerTiles( CRConnection *conn, char *response );
int crMothershipGetServerDisplayTiles( CRConnection *conn, char *response );
int crMothershipRequestTileLayout( CRConnection *conn, char *response, int muralWidth, int muralHeight );

/* App node functions */
int crMothershipGetFakerParam( CRConnection *conn, char *response, const char *param );

/* Server / App node functions */
int crMothershipGetRank( CRConnection *conn, char *response );

/* CRUT functions */
void crMothershipGetCRUTServer( CRConnection *conn, char *response );
void crMothershipGetCRUTClients( CRConnection *conn, char *response );
int crMothershipGetCRUTServerParam( CRConnection *conn, char *response, const char *param );


#ifdef __cplusplus
}
#endif

#endif /* CR_MOTHERSHIP_H */

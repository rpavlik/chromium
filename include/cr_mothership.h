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

CRConnection *crMothershipConnect( void );
int crMothershipSendString( CRConnection *conn, char *response_buf, char *str, ... );
int crMothershipReadResponse( CRConnection *conn, void *buf );
void crMothershipDisconnect( CRConnection *conn );

void crMothershipIdentifySPU( CRConnection *conn, int spu );
void crMothershipIdentifyOpenGL( CRConnection *conn, char *response, char *app_id );
void crMothershipIdentifyFaker( CRConnection *conn, char *response );
void crMothershipIdentifyServer( CRConnection *conn, char *response );

void crMothershipGetStartdir( CRConnection *conn, char *response );

void crMothershipSetParam( CRConnection *conn, const char *param, const char *value );
int crMothershipGetParam( CRConnection *conn, const char *param, char *value );

int crMothershipGetServerParam( CRConnection *conn, char *response, const char *param, ...);
int crMothershipGetSPUParam( CRConnection *conn, char *response, const char *param, ...);
int crMothershipSetSPUParam( CRConnection *conn, const char *param, const char *value);
int crMothershipGetNamedSPUParam( CRConnection *conn, int spu_id, const char *param, char *response);
void crMothershipGetClients( CRConnection *conn, char *response );
void crMothershipGetServers( CRConnection *conn, char *response );
int crMothershipGetClientDLL( CRConnection *conn, char *response );
int crMothershipGetSPUDir( CRConnection *conn, char *response );
void crMothershipGetMTU( CRConnection *conn, char *response );
int crMothershipGetTiles( CRConnection *conn, char *response, int server_num );
int crMothershipGetServerTiles( CRConnection *conn, char *response );

#ifdef __cplusplus
}
#endif

#endif /* CR_MOTHERSHIP_H */

#ifndef CR_MOTHERSHIP_H
#define CR_MOTHERSHIP_H

#include "cr_net.h"

CRConnection *crMothershipConnect( void );
int crMothershipSendString( CRConnection *conn, char *response_buf, char *str, ... );
int crMothershipReadResponse( CRConnection *conn, void *buf );
void crMothershipDisconnect( CRConnection *conn );

void crMothershipIdentifySPU( CRConnection *conn, int spu );
void crMothershipIdentifyOpenGL( CRConnection *conn, char *response );
void crMothershipIdentifyFaker( CRConnection *conn, char *response );
void crMothershipIdentifyServer( CRConnection *conn, char *response );

void crMothershipGetStartdir( CRConnection *conn, char *response );
int crMothershipServerParam( CRConnection *conn, char *response, char *param, ...);
int crMothershipSPUParam( CRConnection *conn, char *response, char *param, ...);
void crMothershipGetClients( CRConnection *conn, char *response );

#endif /* CR_MOTHERSHIP_H */

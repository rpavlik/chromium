#ifndef CR_MOTHERSHIP_H
#define CR_MOTHERSHIP_H

#include "cr_net.h"

CRConnection *crMothershipConnect( void );
int crMothershipSendString( CRConnection *conn, char *response_buf, char *str, ... );
int crMothershipReadResponse( CRConnection *conn, void *buf );
void crMothershipDisconnect( CRConnection *conn );

#endif /* CR_MOTHERSHIP_H */

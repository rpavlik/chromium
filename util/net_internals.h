#ifndef NET_INTERNALS_H
#define NET_INTERNALS_H

CRConnection *__copy_of_crMothershipConnect( void );
int __copy_of_crMothershipReadResponse( CRConnection *conn, void *buf );
int __copy_of_crMothershipSendString( CRConnection *conn, char *response_buf, char *str, ... );
void __copy_of_crMothershipDisconnect( CRConnection *conn );

#endif /* NET_INTERNALS_H */

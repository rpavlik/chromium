#ifndef CR_URL_H
#define CR_URL_H

int crParseURL( char *url, char *protocol, char *hostname,
				unsigned short *port, unsigned short default_port );

#endif /* CR_URL_H */

/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_URL_H
#define CR_URL_H

#ifdef __cplusplus
extern "C" {
#endif

int crParseURL( const char *url, char *protocol, char *hostname,
				unsigned short *port, unsigned short default_port );

#ifdef __cplusplus
}
#endif

#endif /* CR_URL_H */

#ifndef CR_ERROR_H
#define CR_ERROR_H

void crDebug( char *format, ... );
void crWarning( char *format, ... );
void crError( char *format, ... );

#ifndef NDEBUG
#define CRASSERT( PRED ) ((PRED)?(void)0:crError( "Assertion failed: %s, file %s, line %d", #PRED, __FILE__, __LINE__))
#else
#define CRASSERT( PRED ) ((void)0)
#endif

#endif /* CR_ERROR_H */

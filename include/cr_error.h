#ifndef CR_ERROR_H
#define CR_ERROR_H

void CRError( char *format, ... );
#ifndef NDEBUG
#define CRASSERT( PRED ) ((PRED)?(void)0:CRError( "Assertion failed: %s, file %s, line %d", #PRED, __FILE__, __LINE__))
#else
#define CRASSERT( PRED ) ((void)0)
#endif

#endif /* CR_ERROR_H */

#ifndef CR_STRING_H
#define CR_STRING_H

char  *crStrdup( const char *str );
char  *crStrndup( const char *str, unsigned int len );
int    crStrlen( const char *str );
int    crStrcmp( const char *str1, const char *str2 );
int    crStrncmp( const char *str1, const char *str2, int n );
int    crStrcasecmp( const char *str1, const char *str2 );
void   crStrcpy( char *dst, const char *src );
void   crStrncpy( char *dst, const char *src, unsigned int len );
void   crStrcat( char *dst, const char *src );
char  *crStrstr( const char *str, const char *pat );
char  *crStrchr( const char *str, char c );
char  *crStrrchr( const char *str, char c );
int    crStrToInt( const char *str );
char **crStrSplit( const char *str, const char *splitstr );
char **crStrSplitn( const char *str, const char *splitstr, int n );

void   crBytesToString( char *string, int nstring, void *data, int ndata );
void   crWordsToString( char *string, int nstring, void *data, int ndata );

#endif /* CR_STRING_H */

#ifndef CR_STRING_H
#define CR_STRING_H

char *CRStrdup( char *str );
int   CRStrcmp( char *str1, char *str2 );
int   CRStrcasecmp( char *str1, char *str2 );
void  CRStrcpy( char *dst, char *src );
void  CRStrcat( char *dst, char *src );

#endif /* CR_STRING_H */

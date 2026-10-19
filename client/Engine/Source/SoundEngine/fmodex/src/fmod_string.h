#ifndef _SYSTEM_STRING_H
#define _SYSTEM_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

int    FMOD_strlen(const char * s);
char * FMOD_strcpy(char * dest,const char *src);
char * FMOD_strncpy(char * dest, const char *src, int count);
char * FMOD_strcat(char * dest, const char * src);
char * FMOD_strncat(char *dest, const char *src, int count);
char   FMOD_tolower(char in);
char * FMOD_strupr(char *string);
char * FMOD_strlwr(char *string);
int    FMOD_strcmp( const char *string1, const char *string2 );
int    FMOD_strncmp( const char *string1, const char *string2, int len);
int    FMOD_stricmp( const char *string1, const char *string2);
int    FMOD_strnicmp( const char *string1, const char *string2, int len);
char * FMOD_strchr(const char * string, int c);
char * FMOD_strstr(const char * s1,const char * s2);
void * FMOD_memmove(void * dest,const void *src, int count);
int    FMOD_memcmp(const void * cs,const void * ct, int count);

char * FMOD_strdup(const char * src);
char * FMOD_eatwhite(const char *string);
int    FMOD_atoi(const char *s);
int    FMOD_atoi_hex(const char *s);

int FMOD_snprintf(char *str, int size, const char *format, ...);
int FMOD_vsnprintf(char *str, int size, const char *format, va_list arglist);

#define FMOD_isspace(j) (j==' ' || j=='\t' || j=='\n')
#define FMOD_isdigit(j) (j>='0' && j<='9')
#define FMOD_ishexdigit(j) (((j) >= '0' && (j) <= '9') || ((j) >= 'A' && (j) <= 'F') || ((j) >= 'a' && (j) <= 'f'))
#define FMOD_isalpha(j) ((((j) >= 'A') && ((j) <= 'Z')) || (((j) >= 'a') && ((j) <= 'z')))

#define FMOD_STRING_MAXPATHLEN 256
#define FMOD_STRING_MAXNAMELEN 256

#ifdef __cplusplus
}
#endif

#endif


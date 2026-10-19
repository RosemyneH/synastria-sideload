#ifndef _SYSTEM_STRINGW_H
#define _SYSTEM_STRINGW_H

#ifdef __cplusplus
extern "C" {
#endif

int     FMOD_strlenW(const short * s);
short * FMOD_strcpyW(short * dest,const short *src);
short * FMOD_strncpyW(short * dest, const short *src, int count);
short * FMOD_strcatW(short * dest, const short * src);
short * FMOD_strncatW(short *dest, const short *src, int count);
short   FMOD_tolowerW(short in);
short * FMOD_struprW(short *string);
short * FMOD_strlwrW(short *string);
int     FMOD_strcmpW( const short *string1, const short *string2 );
int     FMOD_strncmpW( const short *string1, const short *string2, int len);
int     FMOD_stricmpW( const short *string1, const short *string2);
int     FMOD_strnicmpW( const short *string1, const short *string2, int len);
short * FMOD_strstrW(const short * s1,const short * s2);
short * FMOD_strdupW(const short * src);
short * FMOD_eatwhiteW(const short *string);
int     FMOD_atoiW(const short *s);
char *  FMOD_wtoa(short *s);

#define FMOD_isspaceW(j) (j==L' ' || j==L'\t' || j==L'\n')
#define FMOD_isdigitW(j) (j>=L'0' && j<=L'9')
#define FMOD_isalphaW(j) ((((j) >= L'A') && ((j) <= L'Z')) || (((j) >= L'a') && ((j) <= L'z')))

#ifdef __cplusplus
}
#endif

#endif


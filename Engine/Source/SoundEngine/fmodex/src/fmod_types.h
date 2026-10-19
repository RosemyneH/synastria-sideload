#ifndef _FMOD_TYPES_H
#define _FMOD_TYPES_H

#include "fmod_settings.h"

/*
    Error-checking macro
*/
#define CHECK_RESULT(_x) { FMOD_RESULT _result = (_x); if (_result != FMOD_OK) { return _result; } }

/*
    Structure packing
*/

#if defined(PLATFORM_PS2) || defined(PLATFORM_PSP)
    #define FMOD_PACKED             __attribute__((packed)) /* gcc packed */
    #define FMOD_PACKED_INTERNAL    __attribute__((packed)) /* gcc packed */
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_PS3) || defined(PLATFORM_SOLARIS) || (defined(__GNUC__) && defined(WIN32))
    #define FMOD_PACKED             __attribute__((packed)) /* gcc packed */
    #define FMOD_PACKED_INTERNAL    /* dummy */
#else 
    #define FMOD_PACKED             /* dummy */
    #define FMOD_PACKED_INTERNAL    /* dummy */
#endif

#if defined(PLATFORM_PS3)
    #define FMOD_ALIGN16(__var)  __var __attribute__((aligned(16)))
    #define FMOD_ALIGN128(__var) __var __attribute__((aligned(128)))
    #define FMOD_PPCALIGN16(__var)  __var __attribute__((aligned(16)))
    #define FMOD_PPCALIGN128(__var) __var __attribute__((aligned(128)))
    #define FMOD_RESTRICT
#elif (defined(PLATFORM_XENON) || defined(PLATFORM_WINDOWS) || defined(PLATFORM_XBOX))
    #define FMOD_ALIGN16(__var)  __declspec(align(16)) __var
    #define FMOD_ALIGN128(__var) __declspec(align(128)) __var
    #define FMOD_PPCALIGN16(__var)  __var
    #define FMOD_PPCALIGN128(__var) __var
    #define FMOD_RESTRICT __restrict
#elif (defined(PLATFORM_IPHONE) || defined(PLATFORM_MAC))
    #define FMOD_ALIGN16(__var) __var
    #define FMOD_ALIGN128(__var) __var
    #define FMOD_PPCALIGN16(__var) __var
    #define FMOD_PPCALIGN128(__var) __var
    #define FMOD_RESTRICT __restrict
#else
    #define FMOD_ALIGN16(__var) __var
    #define FMOD_ALIGN128(__var) __var
    #define FMOD_PPCALIGN16(__var) __var
    #define FMOD_PPCALIGN128(__var) __var
    #define FMOD_RESTRICT
#endif

#define FMOD_ALIGNPOINTER(_ptr, _align) ((((FMOD_UINT_NATIVE)(_ptr)) + ((_align) - 1)) & ~((_align) - 1))
#define FMOD_MEMBER_EXISTS(__x, __y) (__x && ((int)(__x->size) >= (((char *)(&__x->__y) - (char *)__x) + sizeof(__x->__y))))

/*
    Custom platform specific types.
*/
#if defined(PLATFORM_WINDOWS) && !defined(__MINGW32__) || defined(PLATFORM_CE) || defined(PLATFORM_XBOX) || defined(PLATFORM_XENON)

    typedef signed __int64   FMOD_SINT64;
    typedef unsigned __int64 FMOD_UINT64;

    #define FMOD_INLINE __forceinline

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MAC) || defined(PLATFORM_IPHONE) || defined(PLATFORM_SOLARIS) || defined(__MINGW32__)

    typedef signed long long   FMOD_SINT64;
    typedef unsigned long long FMOD_UINT64;

    #define FMOD_INLINE inline

#elif defined(PLATFORM_PS2)

    #ifdef R5900
        typedef signed long   FMOD_SINT64;
        typedef unsigned long FMOD_UINT64;   
    #elif defined (R3000)
        typedef signed long long FMOD_SINT64;
        typedef unsigned long long FMOD_UINT64;    
    #endif

    #define FMOD_INLINE __inline
 
#elif defined(PLATFORM_GC) || defined(PLATFORM_WII)

    typedef signed long long FMOD_SINT64;
    typedef unsigned long long FMOD_UINT64;

    #define FMOD_INLINE inline

#elif defined(PLATFORM_PSP)

    typedef signed long long FMOD_SINT64;
    typedef unsigned long long FMOD_UINT64;

    #define FMOD_INLINE __inline

    #ifdef __SNC__
        typedef int SNQuad   __attribute__((mode(VF), aligned(16))) ; 
  //      typedef int SNScalar __attribute__((mode(VS))) ;
    #endif

#elif defined(PLATFORM_PS3)

    typedef signed long long FMOD_SINT64;
    typedef unsigned long long FMOD_UINT64;

    #define FMOD_INLINE inline

#else

    #error define a 64bit int and inline type

#endif

#ifdef PLATFORM_64BIT
    typedef FMOD_UINT64 FMOD_UINT_NATIVE;
    typedef FMOD_SINT64 FMOD_SINT_NATIVE;
#else
    typedef unsigned int FMOD_UINT_NATIVE;
    typedef signed int FMOD_SINT_NATIVE;
#endif

typedef unsigned int    FMOD_UFIXED;
typedef signed int      FMOD_SFIXED;

#ifdef FMOD_NO_FPU

    #define FMOD_FRAC            8
    #define FMOD_SCALE           (1<<FMOD_FRAC)

    typedef FMOD_SFIXED            FMOD_FLOAT;
    typedef FMOD_UFIXED            FMOD_UFLOAT;
    typedef FMOD_SINT64            FMOD_FLOAT64;
    typedef FMOD_UINT64            FMOD_UFLOAT64;
    #define FMOD_SCALEUP(_x)     ((FMOD_FLOAT)  (_x) << FMOD_FRAC)
    #define FMOD_SCALEUP64(_x)   ((FMOD_FLOAT64)(_x) << FMOD_FRAC)
    #define FMOD_SCALEDOWN(_x)   ((FMOD_FLOAT)  (_x) >> FMOD_FRAC)
    #define FMOD_SCALEDOWN64(_x) ((FMOD_FLOAT64)(_x) >> FMOD_FRAC)
    #define FMOD_NEARLYZERO      1
  
    #define FMOD_FMUL(_a, _b)  (((_a) * (_b)) >> FMOD_FRAC)

#else

    typedef float                 FMOD_FLOAT;
    typedef float                 FMOD_UFLOAT;
    typedef float                 FMOD_FLOAT64;
    typedef float                 FMOD_UFLOAT64;
    #define FMOD_SCALEUP(_x)     (FMOD_FLOAT)(_x)
    #define FMOD_SCALEUP64(_x)   (FMOD_FLOAT64)(_x)
    #define FMOD_SCALEDOWN(_x)   (FMOD_FLOAT)(_x)
    #define FMOD_SCALEDOWN64(_x) (FMOD_FLOAT64)(_x)
    #define FMOD_NEARLYZERO      0.00001f

    #define FMOD_FMUL(_a, _b)    ((_a) * (_b))

#endif

#define FMOD_SWAPENDIAN_WORD(_x)  ((((unsigned short)(_x) & 0xFF00) >>  8) | \
                                   (((unsigned short)(_x) & 0x00FF) <<  8))

#define FMOD_SWAPENDIAN_DWORD(_x) ((((unsigned int)(_x) & 0x000000FF) << 24) | \
                                   (((unsigned int)(_x) & 0x0000FF00) <<  8) | \
                                   (((unsigned int)(_x) & 0x00FF0000) >>  8) | \
                                   (((unsigned int)(_x) & 0xFF000000) >> 24))

#define FMOD_SWAPENDIAN_QWORD(_x) ((((FMOD_UINT64)(_x) & 0x00000000000000FFULL) << 56) | \
                                   (((FMOD_UINT64)(_x) & 0x000000000000FF00ULL) << 40) | \
                                   (((FMOD_UINT64)(_x) & 0x0000000000FF0000ULL) << 24) | \
                                   (((FMOD_UINT64)(_x) & 0x00000000FF000000ULL) <<  8) | \
                                   (((FMOD_UINT64)(_x) & 0x000000FF00000000ULL) >>  8) | \
                                   (((FMOD_UINT64)(_x) & 0x0000FF0000000000ULL) >> 24) | \
                                   (((FMOD_UINT64)(_x) & 0x00FF000000000000ULL) >> 40) | \
                                   (((FMOD_UINT64)(_x) & 0xFF00000000000000ULL) >> 56))

#define FMOD_SWAPENDIAN_FLOAT(_x) { unsigned int *__tmp = (unsigned int *)&(_x); *__tmp = FMOD_SWAPENDIAN_DWORD(*__tmp); }

#ifdef PLATFORM_PS2_IOP

/*
	Doesn't allow anonymous unions.  Just fool it for now to make headers compile, because IOP doesnt use this stuff (software mixer etc).
*/
typedef struct
{
    unsigned int    mHi;
    unsigned int    mLo;
} FMOD_UINT64P;

typedef struct
{
    signed int      mHi;
    unsigned int    mLo;
} FMOD_SINT64P;

#else

typedef union
{
    FMOD_UINT64     mValue;

    #ifdef PLATFORM_ENDIAN_LITTLE
    struct 
    {
        unsigned int    mLo;
        unsigned int    mHi;
    };
    #else
    struct 
    {
        unsigned int    mHi;
        unsigned int    mLo;
    };
    #endif
} FMOD_UINT64P;

typedef union
{
    FMOD_SINT64     mValue;
    #ifdef PLATFORM_ENDIAN_LITTLE
    struct 
    {
        unsigned int    mLo;
        signed int      mHi;
    };
    #else
    struct 
    {
        signed int      mHi;
        unsigned int    mLo;
    };
    #endif
} FMOD_SINT64P;

#endif

typedef struct
{
    float re;
    float im;
} FMOD_COMPLEX;

typedef struct
{
    unsigned int   Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} FMOD_GUID;

#ifdef FMOD_SUPPORT_PRAGMAPACK
#pragma pack(1)
#endif

typedef struct
{
    unsigned char val[3];
} FMOD_PACKED  FMOD_INT24;

#ifdef FMOD_SUPPORT_PRAGMAPACK
#pragma pack()
#endif

#define FMOD_ABS(_x)     ((_x) < 0 ? -(_x) : (_x))
#define FMOD_MAX(_a, _b) ((_a) > (_b) ? (_a) : (_b))
#define FMOD_MIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))

#define FMOD_PI    (3.14159265358979323846f)
#define FMOD_PI2   (3.14159265358979323846f * 2.0f)
#define FMOD_PI_2  (3.14159265358979323846f / 2.0f)
#define FMOD_SQRT2 1.41421356237309504880f
#define FMOD_LOG2  0.693147180559945309417f

#define FMOD_ENCRYPT(x, y) { unsigned char __tmp = (x) ^ (y); \
                             (x) = 0;                       \
                             (x) |= (__tmp & 0x01) << 7;    \
                             (x) |= (__tmp & 0x02) << 5;    \
                             (x) |= (__tmp & 0x04) << 3;    \
                             (x) |= (__tmp & 0x08) << 1;    \
                             (x) |= (__tmp & 0x10) >> 1;    \
                             (x) |= (__tmp & 0x20) >> 3;    \
                             (x) |= (__tmp & 0x40) >> 5;    \
                             (x) |= (__tmp & 0x80) >> 7; }

#define FMOD_DECRYPT(x, y) { unsigned char __tmp = (x) ^ (y);   \
                             (x) = 0;                           \
                             (x) |= (__tmp & 0x01) << 7;        \
                             (x) |= (__tmp & 0x02) << 5;        \
                             (x) |= (__tmp & 0x04) << 3;        \
                             (x) |= (__tmp & 0x08) << 1;        \
                             (x) |= (__tmp & 0x10) >> 1;        \
                             (x) |= (__tmp & 0x20) >> 3;        \
                             (x) |= (__tmp & 0x40) >> 5;        \
                             (x) |= (__tmp & 0x80) >> 7; }

#if defined(PLATFORM_PS3) || defined(PLATFORM_PS2) || defined(PLATFORM_PSP)

    #if defined(PLATFORM_PS3) || defined(PLATFORM_PS2_EE) || defined (PLATFORM_PSP)
    #include <math.h>
    #endif

    #ifdef PLATFORM_PSP

        #include <libvfpu.h>

        #define FMOD_SIN sceVfpuScalarSin
        #define FMOD_COS(_s_) FMOD_SIN(_s_ + FMOD_PI_2)
        #define FMOD_SQRT  sceVfpuScalarSqrt
    #else
        #define FMOD_SIN   sinf
        #define FMOD_COS   cosf
        #define FMOD_SQRT  sqrtf
    #endif

    #define FMOD_SINH  sinhf
    #define FMOD_TAN   tanf
    #define FMOD_POW   powf
    #define FMOD_ACOS  acosf
    #define FMOD_ATAN  atanf
    #define FMOD_EXP   expf
    #define FMOD_LDEXP ldexpf
    #define FMOD_FABS  fabsf
    #define FMOD_LOG   logf
    #define FMOD_LOG10 log10f
    #define FMOD_EXP2(_val) powf(2.0f, _val)
    #define FMOD_FMOD  fmodf
    #define FMOD_FLOOR floorf

#else
    
    #include <math.h>

    #define FMOD_SIN   (float)sin
    #define FMOD_SINH  (float)sinh
    #define FMOD_COS   (float)cos
    #define FMOD_TAN   (float)tan
    #define FMOD_SQRT  (float)sqrt
    #define FMOD_POW   (float)pow
    #define FMOD_ACOS  (float)acos
    #define FMOD_ATAN  (float)atan
    #define FMOD_EXP   (float)exp
    #define FMOD_LDEXP (float)ldexp
    #define FMOD_FABS  fabsf
    #define FMOD_LOG   (float)log
    #define FMOD_LOG10 (float)log10
    #define FMOD_FMOD  (float)fmod
    #define FMOD_FLOOR (float)floor
    
    #ifdef PLATFORM_XENON
    #include <xtl.h>
    FMOD_INLINE float FMOD_EXP2(float val)
    {
        __vector4 v_r, v_a = { val, 1.0f, 1.0f, 1.0f };
        v_r = __vexptefp(v_a);
        return v_r.x;
    }
    #else
    #define FMOD_EXP2(_val) FMOD_POW(2.0f, _val)
    #endif
#endif

#include <stdlib.h>

#define FMOD_SRAND(_seed)   srand(_seed)
#define FMOD_RAND()         rand()

#define FMOD_FUZZYEQ(_a, _b) (FMOD_FABS((_a) - (_b)) < 1e-5)

#if defined(PLATFORM_GC) || defined(PLATFORM_WII)
    
    #ifdef __MWERKS__
        #include <alloca.h>
    #else
        #include <malloc.h>
    #endif

    #define FMOD_alloca(x) alloca(x)

#elif defined(PLATFORM_PS2)

    #ifdef PLATFORM_PS2_EE
        #include <malloc.h>

	    #if defined(__MWERKS__)
            #define FMOD_alloca __alloca
        #else
            #define FMOD_alloca alloca
        #endif
    #endif

#elif defined(PLATFORM_PS3) || (defined(PLATFORM_PSP) && defined(__psp_gcc__)) || defined(PLATFORM_LINUX) || defined(PLATFORM_MAC) || defined(PLATFORM_IPHONE) || defined(PLATFORM_SOLARIS)

    #include <alloca.h>
    #define FMOD_alloca(x) alloca(x)

#else

    #include <malloc.h>
    #define FMOD_alloca alloca

#endif

#if defined(PLATFORM_PSP)
#define FMOD_memset sceVfpuMemset
#define FMOD_memcpy sceVfpuMemcpy
#elif defined(PLATFORM_XENON)
#define FMOD_memset XMemSet
#define FMOD_memcpy XMemCpy
#else
#define FMOD_memset memset
#define FMOD_memcpy memcpy
#endif


#if defined(FMOD_SUPPORT_RTTI)

    #define SAFE_CAST(_type, _obj) dynamic_cast<_type *>(_obj)

#else

    #define SAFE_CAST(_type, _obj) static_cast<_type *>(_obj)

#endif

#if defined(PLATFORM_WINDOWS) && defined(_MSC_VER) && (_MSC_VER < 1500)
    #define vsnprintf _vsnprintf
#endif


#endif

#ifndef _FMOD_OS_MISC
#define _FMOD_OS_MISC

#include "fmod.h"
#include "fmod_types.h"

/*
    ==============================================================================
    These functions must be filled in to enable porting to any platform.
    ==============================================================================
*/


/*
    Memory allocation functions
*/
void           *FMOD_OS_Memory_Alloc(int size, FMOD_MEMORY_TYPE type);
void           *FMOD_OS_Memory_Realloc(void *ptr, int size, FMOD_MEMORY_TYPE type);
void            FMOD_OS_Memory_Free(void *ptr, FMOD_MEMORY_TYPE type);

int             FMOD_OS_GetMemoryUsed();

/*
    File functions
*/
FMOD_RESULT     FMOD_OS_File_Open(const char *name, char *mode, int unicode, unsigned int *filesize, void **handle);
FMOD_RESULT     FMOD_OS_File_Close(void *handle);
FMOD_RESULT     FMOD_OS_File_Read(void *handle, void *buf, unsigned int count, unsigned int *read);
FMOD_RESULT     FMOD_OS_File_Seek(void *handle, unsigned int offset);
FMOD_RESULT     FMOD_OS_File_Cancel(void *handle);
FMOD_RESULT     FMOD_OS_File_DriveStatus();

/*
    Debugging functions
*/
FMOD_RESULT     FMOD_OS_Debug_OutputStr(const char *s);

/*
    Time functions
*/
FMOD_RESULT     FMOD_OS_Time_GetNs(unsigned int *ns);
FMOD_RESULT     FMOD_OS_Time_GetMs(unsigned int *ms);
FMOD_RESULT     FMOD_OS_Time_Sleep(unsigned int ms);

/*
    Thread functions
*/

#if defined(PLATFORM_WINDOWS)
    typedef unsigned int THREAD_RETURNTYPE;
    #define THREAD_RETURN return 0;
    #define THREAD_CALLCONV
#elif defined(PLATFORM_PS2)
    typedef void THREAD_RETURNTYPE;
    #define THREAD_RETURN
    #define THREAD_CALLCONV
#elif defined(PLATFORM_PS3_PPU) 
    #include <sys/ppu_thread.h>
    typedef void THREAD_RETURNTYPE;
    #define THREAD_RETURN sys_ppu_thread_exit(0);
    #define THREAD_CALLCONV
#elif defined(PLATFORM_PSP)
    #include <kernel.h>
    typedef int THREAD_RETURNTYPE;
    #define THREAD_RETURN sceKernelExitDeleteThread(0); return 0; 
    #define THREAD_CALLCONV
#else
    typedef unsigned int THREAD_RETURNTYPE;
    #define THREAD_RETURN return 0;
    #define THREAD_CALLCONV
#endif

#if defined(FMOD_SUPPORT_SIMD)

    #if defined (PLATFORM_WINDOWS) || defined (PLATFORM_LINUX)
        #ifdef __cplusplus
        extern "C" {
        #endif
        int FMOD_OS_SupportsSSE();         
        #ifdef __cplusplus
        }
        #endif

        #define FMOD_OS_SupportsSIMD() (FMOD_OS_SupportsSSE() ? true : false);
    #else
        #define FMOD_OS_SupportsSIMD() true;
    #endif

#else
        #define FMOD_OS_SupportsSIMD() false;
#endif

typedef enum
{
    FMOD_THREAD_PRIORITY_VERYLOW  = -2,
    FMOD_THREAD_PRIORITY_LOW      = -1,
    FMOD_THREAD_PRIORITY_NORMAL   = 0,
    FMOD_THREAD_PRIORITY_HIGH     = 1,
    FMOD_THREAD_PRIORITY_VERYHIGH = 2,
    FMOD_THREAD_PRIORITY_CRITICAL = 3
} FMOD_THREAD_PRIORITY;

FMOD_RESULT     FMOD_OS_Thread_GetCurrentID(FMOD_UINT_NATIVE *id);
FMOD_RESULT     FMOD_OS_Thread_Create(const char *name, THREAD_RETURNTYPE (THREAD_CALLCONV *callback)(void *param), void *param, FMOD_THREAD_PRIORITY priority, void *stack, int stacksize, void **handle);
FMOD_RESULT     FMOD_OS_Thread_Destroy(void *handle);

/*
	CriticalSection functions
*/
typedef struct  FMOD_OS_CRITICALSECTION FMOD_OS_CRITICALSECTION;

FMOD_RESULT     FMOD_OS_CriticalSection_Create(FMOD_OS_CRITICALSECTION **crit, bool memorycrit = false);
FMOD_RESULT     FMOD_OS_CriticalSection_Free(FMOD_OS_CRITICALSECTION *crit);
FMOD_RESULT     FMOD_OS_CriticalSection_Enter(FMOD_OS_CRITICALSECTION *crit);
FMOD_RESULT     FMOD_OS_CriticalSection_Leave(FMOD_OS_CRITICALSECTION *crit);

/*
    Semaphore functions
*/
typedef struct  FMOD_OS_SEMAPHORE FMOD_OS_SEMAPHORE;

FMOD_RESULT     FMOD_OS_Semaphore_Create(FMOD_OS_SEMAPHORE **sema);
FMOD_RESULT     FMOD_OS_Semaphore_Free(FMOD_OS_SEMAPHORE *sema);
FMOD_RESULT     FMOD_OS_Semaphore_Wait(FMOD_OS_SEMAPHORE *sema);
FMOD_RESULT     FMOD_OS_Semaphore_Signal(FMOD_OS_SEMAPHORE *sema, bool interrupt = false);

/*
    Dynamic code loading functions.  Optional.  You only need to support these if the platform allows it.
*/
typedef struct  FMOD_OS_LIBRARY FMOD_OS_LIBRARY;

FMOD_RESULT FMOD_OS_Library_Load(const char *dllname, FMOD_OS_LIBRARY **handle);
FMOD_RESULT FMOD_OS_Library_GetProcAddress(FMOD_OS_LIBRARY *handle, const char *procname, void **address);
FMOD_RESULT FMOD_OS_Library_Free(FMOD_OS_LIBRARY *handle);

/*
    Generic driver functionaliy. Optional. You only need to support these if the platform supports multiple sound devices.
*/
FMOD_RESULT FMOD_OS_CheckDriverList(bool *devicelistchanged);

extern int gSizeofCriticalSection;
extern int gSizeofSemaphore;

#endif

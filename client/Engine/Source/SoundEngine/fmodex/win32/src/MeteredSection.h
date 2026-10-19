/************************************************************
    Module Name: MeteredSection.h
    Author: Dan Chou
    Description: Defines the metered section synchronization object
************************************************************/

#include <windows.h>

#ifndef _METERED_SECTION_H_
#define _METERED_SECTION_H_

#define MAX_METSECT_NAMELEN 128

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define FILEMAPPING

// Shared info needed for metered section
typedef struct _METSECT_SHARED_INFO {
    BOOL   fInitialized;     // Is the metered section initialized?
#ifdef FILEMAPPING
    LONG   lSpinLock;        // Used to gain access to this structure
#endif
    LONG   lThreadsWaiting;  // Count of threads waiting
    LONG   lAvailableCount;  // Available resource count
    LONG   lMaximumCount;    // Maximum resource count
} METSECT_SHARED_INFO, *LPMETSECT_SHARED_INFO;

// The opaque Metered Section data structure
typedef struct _METERED_SECTION {
    HANDLE hEvent;           // Handle to a kernel event object
#ifdef FILEMAPPING
    HANDLE hFileMap;         // Handle to memory mapped file
#else
    METSECT_SHARED_INFO     SharedInfo;
    CRITICAL_SECTION        hCrit;
#endif
    LPMETSECT_SHARED_INFO   lpSharedInfo;
} METERED_SECTION, *LPMETERED_SECTION;

// Interface functions
LPMETERED_SECTION
CreateMeteredSection(LONG lInitialCount, LONG lMaximumCount, LPCTSTR lpName);

#ifndef _WIN32_WCE
LPMETERED_SECTION OpenMeteredSection(LPCTSTR lpName);
#endif

DWORD EnterMeteredSection(LPMETERED_SECTION lpMetSect, 
      DWORD dwMilliseconds);
BOOL LeaveMeteredSection(LPMETERED_SECTION lpMetSect, 
      LONG lReleaseCount, LPLONG lpPreviousCount);
void CloseMeteredSection(LPMETERED_SECTION lpMetSect);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _METERED_SECTION_H_

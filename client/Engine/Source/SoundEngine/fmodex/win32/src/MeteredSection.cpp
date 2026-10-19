/************************************************************
    Module Name: MeteredSection.c
    Author: Dan Chou
    Description: Implements the metered section synchronization object
************************************************************/

#include <windows.h>
#include <tchar.h>
#include "MeteredSection.h"
#include "../../src/fmod_memory.h"

// Internal function declarations
BOOL InitMeteredSection(LPMETERED_SECTION lpMetSect, LONG lInitialCount, 
      LONG lMaximumCount, LPCTSTR lpName, BOOL bOpenOnly);
BOOL CreateMetSectEvent(LPMETERED_SECTION lpMetSect, LPCTSTR lpName, BOOL 
      bOpenOnly);
BOOL CreateMetSectFileView(LPMETERED_SECTION lpMetSect, LONG lInitialCount, 
      LONG lMaximumCount, LPCTSTR lpName, BOOL bOpenOnly);
void GetMeteredSectionLock(LPMETERED_SECTION lpMetSect);
void ReleaseMeteredSectionLock(LPMETERED_SECTION lpMetSect);

/*
 * CreateMeteredSection
 */
// VERIFY: 4a220666 auto
// OFFSET: 0x008E50C0
LPMETERED_SECTION CreateMeteredSection(LONG lInitialCount, 
      LONG lMaximumCount, LPCTSTR lpName)
{
    LPMETERED_SECTION lpMetSect;

    // Verify the parameters
    if ((lMaximumCount < 1)             ||
        (lInitialCount > lMaximumCount) ||
        (lInitialCount < 0)             ||
        ((lpName) && (_tcslen(lpName) > MAX_METSECT_NAMELEN)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    // Allocate memory for the metered section
    lpMetSect = (LPMETERED_SECTION)FMOD_Memory_Alloc(sizeof(METERED_SECTION));

    // If the memory for the metered section was allocated okay, 
    // initialize it
    if (lpMetSect)
    {
        if (!InitMeteredSection(lpMetSect, lInitialCount, 
      lMaximumCount, lpName, FALSE))
        {
            CloseMeteredSection(lpMetSect);
            lpMetSect = NULL;
        }
    }
    return lpMetSect;
}

/*
 * OpenMeteredSection
 */
#ifndef _WIN32_WCE
LPMETERED_SECTION OpenMeteredSection(LPCTSTR lpName)
{
    LPMETERED_SECTION lpMetSect = NULL;

    if (lpName)
    {
        lpMetSect = (LPMETERED_SECTION)FMOD_Memory_Alloc(sizeof(METERED_SECTION));

        // If the memory for the metered section was allocated okay
        if (lpMetSect)
        {
            if (!InitMeteredSection(lpMetSect, 0, 0, lpName, TRUE))
            {
                // Metered section failed to initialize
                CloseMeteredSection(lpMetSect);
                lpMetSect = NULL;
            }
        }
    }
    return lpMetSect;
}
#endif

/*
 * EnterMeteredSection
 */
// VERIFY: 229a57d6 manual
// OFFSET: 0x008E4F40
DWORD EnterMeteredSection(LPMETERED_SECTION lpMetSect, 
      DWORD dwMilliseconds)
{
    while (TRUE)
    {
        GetMeteredSectionLock(lpMetSect);

        // We have access to the metered section, everything we 
        // do now will be atomic
        if (lpMetSect->lpSharedInfo->lAvailableCount >= 1)
        {
            lpMetSect->lpSharedInfo->lAvailableCount--;
            ReleaseMeteredSectionLock(lpMetSect);
            return WAIT_OBJECT_0;
        }

        // Couldn't get in. Wait on the event object
        lpMetSect->lpSharedInfo->lThreadsWaiting++;
        ResetEvent(lpMetSect->hEvent);
        ReleaseMeteredSectionLock(lpMetSect);
        if (WaitForSingleObject(lpMetSect->hEvent, 
         dwMilliseconds) == WAIT_TIMEOUT)
        {
            return WAIT_TIMEOUT;
        }
    }
}

/*
 * LeaveMeteredSection
 */
// VERIFY: 17723da2 manual
// OFFSET: 0x008E4FD0
BOOL LeaveMeteredSection(LPMETERED_SECTION lpMetSect, LONG lReleaseCount, 
         LPLONG lpPreviousCount)
{
    int iCount;
    GetMeteredSectionLock(lpMetSect);

    // Save the old value if they want it
    if (lpPreviousCount)
    {
        *lpPreviousCount = lpMetSect->lpSharedInfo->lAvailableCount;
    }

    // We have access to the metered section, 
    // everything we do now will be atomic
    if ((lReleaseCount < 0) ||
        (lpMetSect->lpSharedInfo->lAvailableCount+lReleaseCount >
         lpMetSect->lpSharedInfo->lMaximumCount))
    {
        ReleaseMeteredSectionLock(lpMetSect);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    lpMetSect->lpSharedInfo->lAvailableCount += lReleaseCount;
    
    // Set the event the appropriate number of times
    lReleaseCount = 
      min(lReleaseCount,lpMetSect->lpSharedInfo->lThreadsWaiting);
    if (lpMetSect->lpSharedInfo->lThreadsWaiting)
    {
        for (iCount=0; iCount < lReleaseCount ; iCount++)
        {
            lpMetSect->lpSharedInfo->lThreadsWaiting--;
            SetEvent(lpMetSect->hEvent);
        }
    }
    ReleaseMeteredSectionLock(lpMetSect);
    return TRUE;
}

/*
 * CloseMeteredSection
 */
// VERIFY: 942dd164 auto
// OFFSET: 0x008E4D60
void CloseMeteredSection(LPMETERED_SECTION lpMetSect)
{
    if (lpMetSect)
    {
        // Clean up
        #ifdef FILEMAPPING
        if (lpMetSect->lpSharedInfo) UnmapViewOfFile(lpMetSect->lpSharedInfo);
        if (lpMetSect->hFileMap) CloseHandle(lpMetSect->hFileMap);
        #else
        DeleteCriticalSection(&lpMetSect->hCrit);
        #endif

        if (lpMetSect->hEvent) CloseHandle(lpMetSect->hEvent);
        FMOD_Memory_Free(lpMetSect);
    }
}

/*
 * InitMeteredSection
 */
// OFFSET: 0x008E5070
BOOL InitMeteredSection(LPMETERED_SECTION lpMetSect, 
         LONG lInitialCount, LONG lMaximumCount,
                        LPCTSTR lpName, BOOL bOpenOnly)
{
    // Try to create the event object
    if (CreateMetSectEvent(lpMetSect, lpName, bOpenOnly))
    {
#ifdef FILEMAPPING
        // Try to create the memory mapped file
        if (CreateMetSectFileView(lpMetSect, lInitialCount, lMaximumCount, lpName, bOpenOnly))
#else        
        lpMetSect->lpSharedInfo = &lpMetSect->SharedInfo;
        lpMetSect->lpSharedInfo->lThreadsWaiting = 0;
        lpMetSect->lpSharedInfo->lAvailableCount = lInitialCount;
        lpMetSect->lpSharedInfo->lMaximumCount = lMaximumCount;
        InitializeCriticalSection(&lpMetSect->hCrit);
        InterlockedExchange((long *)&(lpMetSect->lpSharedInfo->fInitialized), TRUE);
#endif
        {
            return TRUE;
        }
    }

    // Error occured, return FALSE so the caller knows to clean up
    return FALSE;
}

/*
 * CreateMetSectEvent
 */
// OFFSET: 0x008E4DC0
BOOL CreateMetSectEvent(LPMETERED_SECTION lpMetSect, 
         LPCTSTR lpName, BOOL bOpenOnly)
{
    TCHAR sz[MAX_PATH];
    if (lpName)
    {
        //wsprintf(sz, _TEXT("DKC_MSECT_EVT_%s"), lpName);

#ifndef _WIN32_WCE
        if (bOpenOnly)
        {
            lpMetSect->hEvent = OpenEvent(0, FALSE, sz);
        }
        else
        {
#endif
            // Create an auto-reset named event object
            lpMetSect->hEvent = CreateEvent(NULL, FALSE, FALSE, sz);
#ifndef _WIN32_WCE
        }
#endif
    }
    else
    {
        // Create an auto-reset unnamed event object
        lpMetSect->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
    return (lpMetSect->hEvent ? TRUE : FALSE);
}

#ifdef FILEMAPPING
/*
 * CreateMetSectFileView
 */
// OFFSET: 0x008E4E20
BOOL CreateMetSectFileView(LPMETERED_SECTION lpMetSect, 
         LONG lInitialCount, LONG lMaximumCount,
                           LPCTSTR lpName, BOOL bOpenOnly)
{
    TCHAR sz[MAX_PATH];
    DWORD dwLastError; 

    if (lpName)
    {
        //wsprintf(sz, _TEXT("DKC_MSECT_MMF_%s"), lpName);

#ifndef _WIN32_WCE
        if (bOpenOnly)
        {
            lpMetSect->hFileMap = OpenFileMapping(0, FALSE, sz);
        }
        else
        {
#endif
            // Create a named file mapping
            lpMetSect->hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, 
         NULL, PAGE_READWRITE, 0, sizeof(METSECT_SHARED_INFO), sz);
#ifndef _WIN32_WCE
        }
#endif
    }
    else
    {
        // Create an unnamed file mapping
        lpMetSect->hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, 
         NULL, PAGE_READWRITE, 0, sizeof(METSECT_SHARED_INFO), NULL);
    }
 
    // Map a view of the file
    if (lpMetSect->hFileMap)
    {
        dwLastError = GetLastError();
        lpMetSect->lpSharedInfo = (LPMETSECT_SHARED_INFO) 
         MapViewOfFile(lpMetSect->hFileMap, FILE_MAP_WRITE, 0, 0, 0);
        if (lpMetSect->lpSharedInfo)
        {
            if (dwLastError != ERROR_ALREADY_EXISTS)
            {
                lpMetSect->lpSharedInfo->lSpinLock = 0;
                lpMetSect->lpSharedInfo->lThreadsWaiting = 0;
                lpMetSect->lpSharedInfo->lAvailableCount = lInitialCount;
                lpMetSect->lpSharedInfo->lMaximumCount = lMaximumCount;
                InterlockedExchange((long *)&(lpMetSect->lpSharedInfo->fInitialized), TRUE);
            }
            else
            {   // Already exists; wait for it to be initialized by the creator
              while (!lpMetSect->lpSharedInfo->fInitialized) Sleep(0);
            }
            return TRUE;
        }
    }

    return FALSE;
}
#endif

/*
 * GetMeteredSectionLock
 */
// VERIFY: 78301afe auto
// OFFSET: 0x008E4F00
void GetMeteredSectionLock(LPMETERED_SECTION lpMetSect)
{
#ifdef FILEMAPPING
    // Spin and get access to the metered section lock
    while (InterlockedExchange(&(lpMetSect->lpSharedInfo->lSpinLock), 1) != 0)
        Sleep(1);
#else
    EnterCriticalSection(&lpMetSect->hCrit);
#endif
}

/*
 * ReleaseMeteredSectionLock
 */
void ReleaseMeteredSectionLock(LPMETERED_SECTION lpMetSect)
{
#ifdef FILEMAPPING
    InterlockedExchange(&(lpMetSect->lpSharedInfo->lSpinLock), 0);
#else
    LeaveCriticalSection(&lpMetSect->hCrit);
#endif
}
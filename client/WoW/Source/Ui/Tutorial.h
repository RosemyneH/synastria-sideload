#ifndef WOW_SOURCE_UI_TUTORIAL_H
#define WOW_SOURCE_UI_TUTORIAL_H

#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"
#include "WoW/Source/Net/Types.h"

class CDataStore;
struct lua_State;

class CGTutorial {
    public:
    static void SaveFlaggedTutorials();
    static void InitializeGame();
    static void ShutdownGame();
    static void LoadFlaggedTutorials();
    static TSBitField<TSFixedArray<uint32_t> > m_tutorialShown;
    static TSBitField<TSFixedArray<uint32_t> > m_tutorialFlags;

    static void FlagTutorial(uint32_t tutorial);
    static void TriggerTutorial(uint32_t tutorial);
    static int32_t GetNextCompletedTutorial(int32_t tutorial);
    static int32_t GetPrevCompletedTutorial(int32_t tutorial);
    static void ClearTutorials();
    static void ResetTutorials();
    static int32_t OnTutorialFlags(void* param, NETMESSAGE msgId, uint32_t time, CDataStore* msg);
};

void TutorialRegisterScriptFunctions();
void TutorialUnregisterScriptFunctions();

#endif

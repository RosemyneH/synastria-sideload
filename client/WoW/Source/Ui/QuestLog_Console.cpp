#include "QuestLog.h"
#include "WoW/Source/ObjectMgrClient/ObjectMgrClient.h"

// VERIFY: 573f4f5d manual
// OFFSET: 0x005E7170
int32_t UpdateQuestPOIHandler(const char* command, const char* arguments) {
    if (ClntObjMgrGetActivePlayerObj()) {
        CGQuestLog::UpdateQuestPOIFromUserServer();
    }

    return 1;
}

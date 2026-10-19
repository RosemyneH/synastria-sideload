#ifndef WOW_SOURCE_UI_CACCOUNTMSGHEADERS_H
#define WOW_SOURCE_UI_CACCOUNTMSGHEADERS_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"
#include "WoW/Source/Ui/KnowledgeBase.h"

class CAccountMsgHeaders : public CKBPage {
    public:
    /* 0040 */ int32_t m_count;
    /* 0044 */ CKBIterator m_iterator;
    /* 004C */ TSGrowableArray<int32_t> m_opened;

    virtual ~CAccountMsgHeaders();
    virtual void _SendSuccessEvent();
    virtual void _SendFailureEvent();
    int32_t LoadHeaders();
    const char* GetHeaderSubject(int32_t index);
    int32_t GetHeaderPriority(int32_t index);
    int32_t GetMessageId(int32_t index);
    int32_t GetNumUnreadUrgentMsgs();
    int32_t GetIndexHighestPriorityUnreadMsg();
    int32_t GetIndexNextUnreadMsg(int32_t index);
    int32_t SetMsgRead(int32_t index);
};

extern CAccountMsgHeaders g_accountMsgHeaders;

void AccountMsg_RegisterScriptFunctions();
void AccountMsg_UnregisterScriptFunctions();

#endif

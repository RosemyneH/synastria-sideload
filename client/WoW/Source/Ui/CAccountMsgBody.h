#ifndef WOW_SOURCE_UI_CACCOUNTMSGBODY_H
#define WOW_SOURCE_UI_CACCOUNTMSGBODY_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Ui/KnowledgeBase.h"

class CAccountMsgBody : public CKBPage {
    public:
    /* 0040 */ CKBIterator m_iterator;

    virtual ~CAccountMsgBody();
    virtual void _SendSuccessEvent();
    virtual void _SendFailureEvent();
    int32_t LoadBody(int32_t messageId);
};

extern CAccountMsgBody g_accountMsgBody;

#endif

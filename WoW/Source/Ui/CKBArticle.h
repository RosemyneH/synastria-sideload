#ifndef WOW_SOURCE_UI_CKBARTICLE_H
#define WOW_SOURCE_UI_CKBARTICLE_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Ui/KnowledgeBase.h"

class CKBArticle : public CKBPage {
    public:
    CKBArticle();
    int32_t BeginLoading(int32_t articleId, int32_t searchType);
    virtual void _ComputeCachedData();
    virtual void _SendSuccessEvent();
    virtual void _SendFailureEvent();
};

extern CKBArticle g_KnowledgeBaseArticle;

#endif

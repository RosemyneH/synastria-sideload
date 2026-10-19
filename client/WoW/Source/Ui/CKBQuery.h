#ifndef WOW_SOURCE_UI_CKBQUERY_H
#define WOW_SOURCE_UI_CKBQUERY_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Ui/KnowledgeBase.h"

class CKBQuery : public CKBPage {
    public:
    /* 0040 */ int32_t m_numArticleHeaders;
    /* 0044 */ CKBIterator m_articleHeaders;

    static void _AppendMangledSearchQuery(CDynamicString& url, const char* query);

    CKBQuery();
    int32_t BeginLoading(const char* query, int32_t categoryId, int32_t numArticles, int32_t pageNumber);
    XMLNode* GetArticleHeader(int32_t index);
    virtual void _ComputeCachedData();
    virtual void _SendSuccessEvent();
    virtual void _SendFailureEvent();
};

extern CKBQuery g_KnowledgeBaseQuery;

#endif

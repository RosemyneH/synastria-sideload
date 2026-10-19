#ifndef WOW_SOURCE_OBJECT_PAGETEXTCACHE_H
#define WOW_SOURCE_OBJECT_PAGETEXTCACHE_H

#include <cstddef>
#include <cstdint>

class CDataStore;

class PageTextCache {
    public:
    /* 0004 */ char* m_text;
    /* 0008 */ uint32_t m_nextPage;

    virtual ~PageTextCache();

    void Pack(CDataStore* msg);
};

class PageTextCache_C : public PageTextCache {
    public:
    static const uint32_t s_cacheVersion = 1;

    void Unpack(CDataStore* msg);
};

#endif

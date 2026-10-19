#ifndef WOW_SOURCE_OBJECT_ARENATEAMCACHE_H
#define WOW_SOURCE_OBJECT_ARENATEAMCACHE_H

#include <cstddef>
#include <cstdint>

class CDataStore;

class ArenaTeamCache {
    public:
    static const uint32_t s_cacheVersion = 1;

    /* 0000 */ int32_t m_id;
    /* 0004 */ char m_name[96];
    /* 0064 */ uint32_t m_type;
    /* 0068 */ uint32_t m_backgroundColor;
    /* 006C */ uint32_t m_emblemStyle;
    /* 0070 */ uint32_t m_emblemColor;
    /* 0074 */ uint32_t m_borderStyle;
    /* 0078 */ uint32_t m_borderColor;

    void Pack(CDataStore* msg);
    void Unpack(CDataStore* msg);
};

#endif

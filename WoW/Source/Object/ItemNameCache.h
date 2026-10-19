#ifndef WOW_SOURCE_OBJECT_ITEMNAMECACHE_H
#define WOW_SOURCE_OBJECT_ITEMNAMECACHE_H

#include <cstddef>
#include <cstdint>

class CDataStore;

class ItemNameCache_C {
    public:
    static const uint32_t s_cacheVersion = 1;

    /* 0000 */ char* m_name;
    /* 0004 */ uint32_t m_inventoryType;

    ~ItemNameCache_C();
    void Pack(CDataStore* msg);
    void Unpack(CDataStore* msg);
};

#endif

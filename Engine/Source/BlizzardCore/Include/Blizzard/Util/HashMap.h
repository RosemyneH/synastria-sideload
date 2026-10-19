#ifndef ENGINE_SOURCE_BLIZZARDCORE_INCLUDE_BLIZZARD_UTIL_HASHMAP_H
#define ENGINE_SOURCE_BLIZZARDCORE_INCLUDE_BLIZZARD_UTIL_HASHMAP_H

#include <algorithm>
#include <cstdint>
#include "Engine/Source/Storm/H/storm.h"

namespace Blizzard {
namespace Util {

template <class K, class V>
class HashMap {
    public:
    struct Entry {
        /* 0000 */ K m_key;
        /* 0008 */ V m_value;
    };

    /* 0000 */ Entry* m_entries;
    /* 0004 */ uint32_t m_count;
    /* 0008 */ uint32_t m_capacity;

    ~HashMap();
    uint32_t CopyEntries(const Entry* entries, uint32_t count);
    bool Find(const K& key, uint32_t* index);
    bool Get(const K& key, V* value);
    bool Insert(const K& key, const V& value);
    void Resize(uint32_t capacity);
};

}
}

// INLINED
template <class K, class V>
Blizzard::Util::HashMap<K, V>::~HashMap() {
    for (uint32_t i = 0; i < this->m_capacity; i++) {
        if (this->m_entries[i].m_key.m_lo) {
            this->m_entries[i].m_key.m_lo = 0;
            this->m_entries[i].m_key.m_hi = 0;
        }
    }

    this->m_count = 0;

    if (this->m_entries) {
        SMemFree(this->m_entries, __FILE__, __LINE__, 0);
        this->m_entries = nullptr;
        this->m_capacity = 0;
    }
}

// VERIFY: 23f50510 manual
// OFFSET: 0x00433BA0
template <class K, class V>
bool Blizzard::Util::HashMap<K, V>::Find(const K& key, uint32_t* index) {
    if (!this->m_capacity) {
        return false;
    }

    for (*index = key.m_lo & (this->m_capacity - 1); this->m_entries[*index].m_key.m_lo; *index = (*index + 1) & (this->m_capacity - 1)) {
        if (this->m_entries[*index].m_key.m_lo == key.m_lo && this->m_entries[*index].m_key.m_hi == key.m_hi) {
            return true;
        }
    }

    return false;
}

// VERIFY: d994337d manual
// OFFSET: 0x00435EA0
template <class K, class V>
bool Blizzard::Util::HashMap<K, V>::Get(const K& key, V* value) {
    uint32_t index;

    if (!this->Find(key, &index)) {
        return false;
    }

    *value = this->m_entries[index].m_value;

    return true;
}

// VERIFY: caae72cc manual
// OFFSET: 0x00433EC0
template <class K, class V>
uint32_t Blizzard::Util::HashMap<K, V>::CopyEntries(const Entry* entries, uint32_t count) {
    uint32_t copied = 0;

    for (uint32_t i = 0; i < count; i++) {
        if (entries[i].m_key.m_lo) {
            uint32_t index = 0;
            this->Find(entries[i].m_key, &index);
            this->m_entries[index] = entries[i];
            copied++;
        }
    }

    return copied;
}

// VERIFY: c7fc19cb auto
// OFFSET: 0x0043A400
template <class K, class V>
bool Blizzard::Util::HashMap<K, V>::Insert(const K& key, const V& value) {
    if (this->m_count >= (3 * this->m_capacity) >> 2) {
        this->Resize(std::max(2 * this->m_capacity, 8u));
    }

    uint32_t index = 0;

    if (this->Find(key, &index)) {
        return false;
    }

    this->m_count++;
    this->m_entries[index].m_key = key;
    this->m_entries[index].m_value = value;

    return true;
}

// VERIFY: 28591a4c manual
// OFFSET: 0x004362C0
template <class K, class V>
void Blizzard::Util::HashMap<K, V>::Resize(uint32_t capacity) {
    Entry* old = this->m_entries;
    uint32_t oldCapacity = this->m_capacity;

    this->m_entries = static_cast<Entry*>(SMemAlloc(sizeof(Entry) * capacity, __FILE__, __LINE__, 0));
    this->m_capacity = capacity;

    for (uint32_t i = 0; i < capacity; i++) {
        this->m_entries[i].m_key.m_lo = 0;
        this->m_entries[i].m_key.m_hi = 0;
    }

    if (!old) {
        return;
    }

    this->CopyEntries(old, oldCapacity);

    for (uint32_t i = 0; i < oldCapacity; i++) {
        if (old[i].m_key.m_lo) {
            old[i].m_key.m_lo = 0;
            old[i].m_key.m_hi = 0;
        }
    }

    SMemFree(old, __FILE__, __LINE__, 0);
}

#endif

#ifndef WOW_SOURCE_BATTLENET_BATTLENETMAP_H
#define WOW_SOURCE_BATTLENET_BATTLENETMAP_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include "WoW/Source/Battlenet/Battlenet.h"

namespace Battlenet {
class String {
    public:
    struct Block {
        /* 0000 */ uint32_t m_refCount;
        /* 0004 */ uint32_t m_capacity;
    };

    /* 0000 */ Block* m_block;
    /* 0004 */ const char* m_string;

    String();
    String(const char* string);
    String(const String& other);
    ~String();
    String& operator=(const String& other);
    void Release();
    const char* GetString() const;
};

class MapKey {
    public:
    /* 0000 */ uint32_t m_hash;
    /* 0004 */ String m_value;

    MapKey();
    MapKey(const String& value);
    MapKey(const char* value);
    ~MapKey();
    MapKey& operator=(const MapKey& other);
    bool operator==(const MapKey& other) const;
};

template <class V>
class Map {
    public:
    class Entry {
        public:
        /* 0000 */ MapKey m_key;
        /* 000C */ V m_value;

        Entry();
        Entry(const MapKey& key);
    };

    /* 0000 */ Entry* m_entries;
    /* 0004 */ uint32_t m_used;
    /* 0008 */ uint32_t m_capacity;

    Map();
    ~Map();
    void Clear();
    bool Find(const MapKey& key, uint32_t& index) const;
    bool Next(uint32_t& iterator, MapKey& key, V& value) const;
    uint32_t Reinsert(const Entry* entries, uint32_t count);
    bool Remove(const MapKey& key, V& value);
    void Resize(uint32_t capacity);
    bool Shrink();
    V& operator[](const MapKey& key);
};
}

// INLINED
template <class V>
Battlenet::Map<V>::Entry::Entry() {
}

// INLINED
template <class V>
Battlenet::Map<V>::Entry::Entry(const MapKey& key) : m_key(key), m_value() {
}

// INLINED
template <class V>
Battlenet::Map<V>::Map() {
    this->m_entries = nullptr;
    this->m_used = 0;
    this->m_capacity = 0;
}

// INLINED
template <class V>
Battlenet::Map<V>::~Map() {
    this->Clear();
}

// VERIFY: 51ae2a94 manual
// OFFSET: 0x009C06E0
template <class V>
void Battlenet::Map<V>::Clear() {
    for (uint32_t i = 0; i < this->m_capacity; i++) {
        if (this->m_entries[i].m_key.m_hash) {
            this->m_entries[i].~Entry();
        }
    }

    this->m_used = 0;

    if (this->m_entries) {
        Battlenet::g_allocator->Free(this->m_entries);
        this->m_entries = nullptr;
        this->m_capacity = 0;
    }
}

// VERIFY: c3f75e70 manual
// OFFSET: 0x009C02F0
template <class V>
bool Battlenet::Map<V>::Find(const MapKey& key, uint32_t& index) const {
    if (!this->m_capacity) {
        return false;
    }

    index = key.m_hash & (this->m_capacity - 1);

    while (this->m_entries[index].m_key.m_hash) {
        if (this->m_entries[index].m_key == key) {
            return true;
        }

        index = (index + 1) & (this->m_capacity - 1);
    }

    return false;
}

// VERIFY: 4da873ed manual
// OFFSET: 0x009C0420
template <class V>
bool Battlenet::Map<V>::Next(uint32_t& iterator, MapKey& key, V& value) const {
    if (iterator >= this->m_capacity) {
        return false;
    }

    while (!this->m_entries[iterator].m_key.m_hash) {
        if (++iterator >= this->m_capacity) {
            return false;
        }
    }

    key = this->m_entries[iterator].m_key;
    value = this->m_entries[iterator].m_value;
    iterator++;
    return true;
}

// VERIFY: 5e206c99 manual
// OFFSET: 0x009C0390
template <class V>
uint32_t Battlenet::Map<V>::Reinsert(const Entry* entries, uint32_t count) {
    uint32_t inserted = 0;

    for (; count; count--, entries++) {
        uint32_t index;

        if (entries->m_key.m_hash && !this->Find(entries->m_key, index)) {
            new (&this->m_entries[index]) Entry(*entries);
            inserted++;
        }
    }

    return inserted;
}

// VERIFY: 1062102e manual
// OFFSET: 0x009C0860
template <class V>
bool Battlenet::Map<V>::Remove(const MapKey& key, V& value) {
    uint32_t index;

    if (!this->Find(key, index)) {
        return false;
    }

    value = this->m_entries[index].m_value;
    this->m_entries[index].~Entry();
    this->m_used--;

    if (this->Shrink()) {
        return true;
    }

    index = (index + 1) & (this->m_capacity - 1);

    while (this->m_entries[index].m_key.m_hash) {
        uint32_t target;

        if (!this->Find(this->m_entries[index].m_key, target)) {
            new (&this->m_entries[target]) Entry(this->m_entries[index]);
            this->m_entries[index].~Entry();
        }

        index = (index + 1) & (this->m_capacity - 1);
    }

    return true;
}

// VERIFY: 5d6163b0 manual
// OFFSET: 0x009C0490
template <class V>
void Battlenet::Map<V>::Resize(uint32_t capacity) {
    uint32_t oldCapacity = this->m_capacity;
    Entry* oldEntries = this->m_entries;

    this->m_entries = static_cast<Entry*>(Battlenet::g_allocator->Alloc(sizeof(Entry) * capacity));
    this->m_capacity = capacity;

    for (uint32_t i = 0; i < capacity; i++) {
        new (&this->m_entries[i]) Entry();
    }

    if (oldEntries) {
        this->Reinsert(oldEntries, oldCapacity);

        for (uint32_t i = 0; i < oldCapacity; i++) {
            if (oldEntries[i].m_key.m_hash) {
                oldEntries[i].~Entry();
            }
        }

        Battlenet::g_allocator->Free(oldEntries);
    }
}

// VERIFY: c6cc5a2e auto
// OFFSET: 0x009C0770
template <class V>
bool Battlenet::Map<V>::Shrink() {
    uint32_t capacity = this->m_capacity;

    if (this->m_used >= (3 * capacity) >> 3 || capacity <= 8) {
        return false;
    }

    this->Resize(capacity >> 1 < 8 ? 8 : capacity >> 1);
    return true;
}

// VERIFY: 1d94e297 manual
// OFFSET: 0x009C07C0
template <class V>
V& Battlenet::Map<V>::operator[](const MapKey& key) {
    uint32_t capacity = this->m_capacity;

    if (this->m_used >= (3 * capacity) >> 2) {
        this->Resize(2 * capacity < 8 ? 8 : 2 * capacity);
    }

    uint32_t index;

    if (!this->Find(key, index)) {
        this->m_used++;
        new (&this->m_entries[index]) Entry(key);
    }

    return this->m_entries[index].m_value;
}

#endif

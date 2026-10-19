#include "WoW/Source/Battlenet/BattlenetMap.h"

// INLINED
Battlenet::String::String() {
    this->m_block = nullptr;
    this->m_string = nullptr;
}

// VERIFY: 25c8cd5e manual
// OFFSET: 0x009BFF00
Battlenet::String::String(const char* string) {
    if (!string) {
        this->m_block = nullptr;
        this->m_string = nullptr;
        return;
    }

    uint32_t capacity = (static_cast<uint32_t>(strlen(string)) + 40) & ~31u;
    Block* block = static_cast<Block*>(Battlenet::g_allocator->Alloc(capacity + sizeof(Block)));

    if (block) {
        block->m_refCount = 1;
        block->m_capacity = capacity;
        Battlenet::StringCopy(block + 1, string, capacity);
    }

    this->m_block = block;
    this->m_string = reinterpret_cast<const char*>(block + 1);
}

// INLINED
Battlenet::String::String(const String& other) {
    this->m_block = other.m_block;
    this->m_string = other.m_string;

    if (this->m_block) {
        this->m_block->m_refCount++;
    }
}

// INLINED
Battlenet::String::~String() {
    this->Release();
}

// INLINED
void Battlenet::String::Release() {
    if (this->m_block) {
        if (this->m_block->m_refCount-- == 1) {
            Battlenet::g_allocator->Free(this->m_block);
        }

        this->m_block = nullptr;
    }

    this->m_string = nullptr;
}

// INLINED
Battlenet::String& Battlenet::String::operator=(const String& other) {
    this->Release();
    this->m_block = other.m_block;
    this->m_string = other.m_string;

    if (this->m_block) {
        this->m_block->m_refCount++;
    }

    return *this;
}

// INLINED
const char* Battlenet::String::GetString() const {
    return this->m_string ? this->m_string : "";
}

// INLINED
Battlenet::MapKey::MapKey() {
    this->m_hash = 0;
}

// VERIFY: 3ed3a97e manual
// OFFSET: 0x009C01B0
Battlenet::MapKey::MapKey(const String& value) {
    const char* string = value.GetString();
    uint32_t hash = 0x811C9DC5;

    for (uint8_t c = *string; c; c = *++string) {
        hash = 16777619 * (c ^ hash);
    }

    this->m_hash = hash | 0x80000000;
    this->m_value.m_block = value.m_block;
    this->m_value.m_string = value.m_string;

    if (value.m_block) {
        value.m_block->m_refCount++;
    }
}

// VERIFY: 208e50e4 manual
// OFFSET: 0x009C0210
Battlenet::MapKey::MapKey(const char* value) : MapKey(String(value)) {
}

// VERIFY: 3e7e1881 manual
// OFFSET: 0x009C0260
Battlenet::MapKey::~MapKey() {
    this->m_hash = 0;
}

// VERIFY: 84dfa24e manual
// OFFSET: 0x009C02A0
Battlenet::MapKey& Battlenet::MapKey::operator=(const MapKey& other) {
    this->m_hash = other.m_hash;
    this->m_value = other.m_value;
    return *this;
}

// INLINED
bool Battlenet::MapKey::operator==(const MapKey& other) const {
    return this->m_hash == other.m_hash && !strcmp(this->m_value.m_string, other.m_value.m_string);
}

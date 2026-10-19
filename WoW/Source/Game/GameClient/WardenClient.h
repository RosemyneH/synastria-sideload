#ifndef WOW_SOURCE_GAME_GAMECLIENT_WARDENCLIENT_H
#define WOW_SOURCE_GAME_GAMECLIENT_WARDENCLIENT_H

#include <cstddef>
#include <cstdint>

class CDataStore;

class CWardenKey {
    public:
    /* 0000 */ uint8_t m_key[16];

    CWardenKey& operator=(const CWardenKey& key);
    bool operator==(const CWardenKey& key);
    uint32_t Hash() const;
};

class WardenCachedModule {
    public:
    static const uint32_t s_cacheVersion = 1;

    /* 0000 */ uint32_t m_size;
    /* 0004 */ uint8_t* m_data;

    WardenCachedModule();
    ~WardenCachedModule();
    WardenCachedModule& operator=(const WardenCachedModule& source);
    void Pack(CDataStore* msg);
    void Unpack(CDataStore* msg);
};

CDataStore& operator<<(CDataStore& msg, const CWardenKey& key);
CDataStore& operator>>(CDataStore& msg, CWardenKey& key);

class WardenModule {
    public:
    virtual void Initialize(const uint8_t* sessionKey, uint32_t keyLength) = 0;
    virtual void Shutdown() = 0;
    virtual void Data(const void* data, uint32_t size, uint32_t* consumed) = 0;
    virtual void Process(uint32_t elapsedMs) = 0;
};

namespace WardenClient {
    struct Id;
    struct Key;

    class OurLibrary {
        public:
        virtual void Data(const void* data, uint32_t size);
        virtual bool ModuleUse(const WardenClient::Id& id, const WardenClient::Key& key);
        virtual void ModuleCache(const WardenClient::Id& id, const void* data, uint32_t size);
        virtual void* MemoryAlloc(uint32_t size);
        virtual void MemoryFree(void* ptr);
        virtual void StateSave(const void* data, uint32_t size);
        virtual bool StateLoad(void* data, uint32_t& size);
    };
}

typedef WardenModule* (__fastcall* WARDENMODULEEXPORT)(WardenClient::OurLibrary* library);

void WardenClient_Destroy();
bool WardenClient_Initialize();
void WardenClient_Process();

#endif

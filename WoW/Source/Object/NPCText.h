#ifndef WOW_SOURCE_OBJECT_NPCTEXT_H
#define WOW_SOURCE_OBJECT_NPCTEXT_H

#include <cstddef>
#include <cstdint>

class CDataStore;

class NPCText {
    public:
    static const uint32_t s_cacheVersion = 1;

    /* 0000 */ float m_probability[8];
    /* 0020 */ char* m_text[8][2];
    /* 0060 */ uint32_t m_language[8];
    /* 0080 */ uint32_t m_emoteDelay[8][3];
    /* 00E0 */ uint32_t m_emote[8][3];

    ~NPCText();
    void Pack(CDataStore* msg);
    void Unpack(CDataStore* msg);
};

#endif

#ifndef WOW_SOURCE_GAME_NAMEGEN_H
#define WOW_SOURCE_GAME_NAMEGEN_H

#include <cstddef>
#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"

struct DictionaryRecord {
    /* 0000 */ char m_str [4];
    /* 0004 */ int32_t m_count;
};

class CNameGen {
    public:
    /* 0000 */ TSFixedArray<const char*> m_names;
    /* 000C */ TSGrowableArray<DictionaryRecord> m_dictionary;
    /* 001C */ int32_t m_dictionaryCount;

    void ClearDictionary();
    void CreateDictionary();
    void GenerateName(char* buffer, uint32_t size);
    void LoadNames(int32_t raceID, int32_t sexID);
};

#endif

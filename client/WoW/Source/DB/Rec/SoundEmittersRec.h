#ifndef WOW_SOURCE_DB_REC_SOUNDEMITTERSREC_H
#define WOW_SOURCE_DB_REC_SOUNDEMITTERSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SoundEmittersRec {
    public:
    static const int32_t s_numColumns = 10;
    static const int32_t s_rowSize = 40;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ float m_position [3];
    /* 0010 */ float m_direction [3];
    /* 001C */ int32_t m_soundEntryAdvancedID;
    /* 0020 */ int32_t m_mapID;
    /* 0024 */ const char* m_name;

    SoundEmittersRec();
    ~SoundEmittersRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

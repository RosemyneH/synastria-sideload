#ifndef WOW_SOURCE_DB_REC_DEATHTHUDLOOKUPSREC_H
#define WOW_SOURCE_DB_REC_DEATHTHUDLOOKUPSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class DeathThudLookupsRec {
    public:
    static const int32_t s_numColumns = 5;
    static const int32_t s_rowSize = 20;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_sizeClass;
    /* 0008 */ int32_t m_terrainTypeSoundID;
    /* 000C */ int32_t m_soundEntryID;
    /* 0010 */ int32_t m_soundEntryIdwater;

    DeathThudLookupsRec();
    ~DeathThudLookupsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

#ifndef WOW_SOURCE_DB_REC_CHARSTARTOUTFITREC_H
#define WOW_SOURCE_DB_REC_CHARSTARTOUTFITREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class CharStartOutfitRec {
    public:
    static const int32_t s_numColumns = 77;
    static const int32_t s_rowSize = 296;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int8_t m_raceID;
    /* 0005 */ int8_t m_classID;
    /* 0006 */ int8_t m_sexID;
    /* 0007 */ int8_t m_outfitID;
    /* 0008 */ int32_t m_itemID [24];
    /* 0068 */ int32_t m_displayItemID [24];
    /* 00C8 */ int32_t m_inventoryType [24];

    CharStartOutfitRec();
    ~CharStartOutfitRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

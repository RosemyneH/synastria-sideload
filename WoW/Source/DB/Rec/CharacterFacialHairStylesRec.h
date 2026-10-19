#ifndef WOW_SOURCE_DB_REC_CHARACTERFACIALHAIRSTYLESREC_H
#define WOW_SOURCE_DB_REC_CHARACTERFACIALHAIRSTYLESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class CharacterFacialHairStylesRec {
    public:
    static const int32_t s_numColumns = 8;
    static const int32_t s_rowSize = 32;
    static const bool s_generatedID = true;

    /* 0000 */ int32_t m_raceID;
    /* 0004 */ int32_t m_sexID;
    /* 0008 */ int32_t m_variationID;
    /* 000C */ int32_t m_geoset [5];
    /* 0020 */ int32_t m_generatedID;

    CharacterFacialHairStylesRec();
    ~CharacterFacialHairStylesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

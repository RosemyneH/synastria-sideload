#ifndef WOW_SOURCE_DB_REC_CHARSECTIONSREC_H
#define WOW_SOURCE_DB_REC_CHARSECTIONSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class CharSectionsRec {
    public:
    static const int32_t s_numColumns = 10;
    static const int32_t s_rowSize = 40;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_raceID;
    /* 0008 */ int32_t m_sexID;
    /* 000C */ int32_t m_baseSection;
    /* 0010 */ const char* m_textureName [3];
    /* 001C */ int32_t m_flags;
    /* 0020 */ int32_t m_variationIndex;
    /* 0024 */ int32_t m_colorIndex;

    CharSectionsRec();
    ~CharSectionsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

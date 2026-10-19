#ifndef WOW_SOURCE_DB_REC_GLYPHPROPERTIESREC_H
#define WOW_SOURCE_DB_REC_GLYPHPROPERTIESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class GlyphPropertiesRec {
    public:
    static const int32_t s_numColumns = 4;
    static const int32_t s_rowSize = 16;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_spellID;
    /* 0008 */ int32_t m_glyphSlotFlags;
    /* 000C */ int32_t m_spellIconID;

    GlyphPropertiesRec();
    ~GlyphPropertiesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

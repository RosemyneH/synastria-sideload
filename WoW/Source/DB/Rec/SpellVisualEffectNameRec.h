#ifndef WOW_SOURCE_DB_REC_SPELLVISUALEFFECTNAMEREC_H
#define WOW_SOURCE_DB_REC_SPELLVISUALEFFECTNAMEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SpellVisualEffectNameRec {
    public:
    static const int32_t s_numColumns = 7;
    static const int32_t s_rowSize = 28;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ const char* m_fileName;
    /* 000C */ float m_areaEffectSize;
    /* 0010 */ float m_scale;
    /* 0014 */ float m_minAllowedScale;
    /* 0018 */ float m_maxAllowedScale;

    SpellVisualEffectNameRec();
    ~SpellVisualEffectNameRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

#ifndef WOW_SOURCE_DB_REC_SPELLDISPELTYPEREC_H
#define WOW_SOURCE_DB_REC_SPELLDISPELTYPEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SpellDispelTypeRec {
    public:
    static const int32_t s_numColumns = 21;
    static const int32_t s_rowSize = 84;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ int32_t m_mask;
    /* 000C */ int32_t m_immunityPossible;
    /* 0010 */ const char* m_internalName;

    SpellDispelTypeRec();
    ~SpellDispelTypeRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

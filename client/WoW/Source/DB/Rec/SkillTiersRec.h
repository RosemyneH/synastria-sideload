#ifndef WOW_SOURCE_DB_REC_SKILLTIERSREC_H
#define WOW_SOURCE_DB_REC_SKILLTIERSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SkillTiersRec {
    public:
    static const int32_t s_numColumns = 33;
    static const int32_t s_rowSize = 132;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_cost [16];
    /* 0044 */ int32_t m_value [16];

    SkillTiersRec();
    ~SkillTiersRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

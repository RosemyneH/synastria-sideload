#ifndef WOW_SOURCE_DB_REC_FACTIONGROUPREC_H
#define WOW_SOURCE_DB_REC_FACTIONGROUPREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class FactionGroupRec {
    public:
    static const int32_t s_numColumns = 20;
    static const int32_t s_rowSize = 80;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_maskID;
    /* 0008 */ const char* m_internalName;
    /* 000C */ const char* m_name;

    FactionGroupRec();
    ~FactionGroupRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

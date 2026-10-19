#ifndef WOW_SOURCE_DB_REC_DURABILITYCOSTSREC_H
#define WOW_SOURCE_DB_REC_DURABILITYCOSTSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class DurabilityCostsRec {
    public:
    static const int32_t s_numColumns = 30;
    static const int32_t s_rowSize = 120;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_weaponSubClassCost [21];
    /* 0058 */ int32_t m_armorSubClassCost [8];

    DurabilityCostsRec();
    ~DurabilityCostsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

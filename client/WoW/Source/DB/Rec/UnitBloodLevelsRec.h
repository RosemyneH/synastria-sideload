#ifndef WOW_SOURCE_DB_REC_UNITBLOODLEVELSREC_H
#define WOW_SOURCE_DB_REC_UNITBLOODLEVELSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class UnitBloodLevelsRec {
    public:
    static const int32_t s_numColumns = 4;
    static const int32_t s_rowSize = 16;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_violencelevel [3];

    UnitBloodLevelsRec();
    ~UnitBloodLevelsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

#ifndef WOW_SOURCE_DB_REC_GAMETABLESREC_H
#define WOW_SOURCE_DB_REC_GAMETABLESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class GameTablesRec {
    public:
    static const int32_t s_numColumns = 3;
    static const int32_t s_rowSize = 12;
    static const bool s_generatedID = true;

    /* 0000 */ const char* m_name;
    /* 0004 */ int32_t m_numRows;
    /* 0008 */ int32_t m_numColumns;
    /* 000C */ int32_t m_generatedID;

    GameTablesRec();
    ~GameTablesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

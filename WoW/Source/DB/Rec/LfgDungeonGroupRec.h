#ifndef WOW_SOURCE_DB_REC_LFGDUNGEONGROUPREC_H
#define WOW_SOURCE_DB_REC_LFGDUNGEONGROUPREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class LfgDungeonGroupRec {
    public:
    static const int32_t s_numColumns = 21;
    static const int32_t s_rowSize = 84;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ int32_t m_orderIndex;
    /* 000C */ int32_t m_parentGroupID;
    /* 0010 */ int32_t m_typeID;

    LfgDungeonGroupRec();
    ~LfgDungeonGroupRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

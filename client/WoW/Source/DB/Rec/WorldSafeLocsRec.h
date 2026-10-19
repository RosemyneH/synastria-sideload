#ifndef WOW_SOURCE_DB_REC_WORLDSAFELOCSREC_H
#define WOW_SOURCE_DB_REC_WORLDSAFELOCSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class WorldSafeLocsRec {
    public:
    static const int32_t s_numColumns = 22;
    static const int32_t s_rowSize = 88;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_continent;
    /* 0008 */ float m_loc [3];
    /* 0014 */ const char* m_areaName;

    WorldSafeLocsRec();
    ~WorldSafeLocsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

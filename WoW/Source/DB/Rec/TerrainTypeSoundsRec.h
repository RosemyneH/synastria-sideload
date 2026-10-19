#ifndef WOW_SOURCE_DB_REC_TERRAINTYPESOUNDSREC_H
#define WOW_SOURCE_DB_REC_TERRAINTYPESOUNDSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class TerrainTypeSoundsRec {
    public:
    static const int32_t s_numColumns = 1;
    static const int32_t s_rowSize = 4;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;

    TerrainTypeSoundsRec();
    ~TerrainTypeSoundsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

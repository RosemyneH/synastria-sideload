#ifndef WOW_SOURCE_DB_REC_ITEMGROUPSOUNDSREC_H
#define WOW_SOURCE_DB_REC_ITEMGROUPSOUNDSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class ItemGroupSoundsRec {
    public:
    static const int32_t s_numColumns = 5;
    static const int32_t s_rowSize = 20;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_sound [4];

    ItemGroupSoundsRec();
    ~ItemGroupSoundsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

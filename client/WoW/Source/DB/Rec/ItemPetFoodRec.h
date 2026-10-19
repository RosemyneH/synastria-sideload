#ifndef WOW_SOURCE_DB_REC_ITEMPETFOODREC_H
#define WOW_SOURCE_DB_REC_ITEMPETFOODREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class ItemPetFoodRec {
    public:
    static const int32_t s_numColumns = 18;
    static const int32_t s_rowSize = 72;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;

    ItemPetFoodRec();
    ~ItemPetFoodRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

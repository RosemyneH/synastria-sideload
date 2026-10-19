#ifndef WOW_SOURCE_DB_REC_TOTEMCATEGORYREC_H
#define WOW_SOURCE_DB_REC_TOTEMCATEGORYREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class TotemCategoryRec {
    public:
    static const int32_t s_numColumns = 20;
    static const int32_t s_rowSize = 80;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ int32_t m_totemCategoryType;
    /* 000C */ int32_t m_totemCategoryMask;

    TotemCategoryRec();
    ~TotemCategoryRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

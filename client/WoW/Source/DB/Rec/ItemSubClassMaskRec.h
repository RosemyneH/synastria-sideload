#ifndef WOW_SOURCE_DB_REC_ITEMSUBCLASSMASKREC_H
#define WOW_SOURCE_DB_REC_ITEMSUBCLASSMASKREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class ItemSubClassMaskRec {
    public:
    static const int32_t s_numColumns = 19;
    static const int32_t s_rowSize = 76;
    static const bool s_generatedID = true;

    /* 0000 */ int32_t m_classID;
    /* 0004 */ int32_t m_mask;
    /* 0008 */ const char* m_name;
    /* 000C */ int32_t m_generatedID;

    ItemSubClassMaskRec();
    ~ItemSubClassMaskRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

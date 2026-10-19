#ifndef WOW_SOURCE_DB_REC_ITEMRANDOMSUFFIXREC_H
#define WOW_SOURCE_DB_REC_ITEMRANDOMSUFFIXREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class ItemRandomSuffixRec {
    public:
    static const int32_t s_numColumns = 29;
    static const int32_t s_rowSize = 116;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;
    /* 0008 */ const char* m_internalName;
    /* 000C */ int32_t m_enchantment [5];
    /* 0020 */ int32_t m_allocationPct [5];

    ItemRandomSuffixRec();
    ~ItemRandomSuffixRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

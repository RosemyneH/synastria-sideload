#ifndef WOW_SOURCE_DB_REC_PAPERDOLLITEMFRAMEREC_H
#define WOW_SOURCE_DB_REC_PAPERDOLLITEMFRAMEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class PaperDollItemFrameRec {
    public:
    static const int32_t s_numColumns = 3;
    static const int32_t s_rowSize = 12;
    static const bool s_generatedID = true;

    /* 0000 */ const char* m_itemButtonName;
    /* 0004 */ const char* m_slotIcon;
    /* 0008 */ int32_t m_slotNumber;
    /* 000C */ int32_t m_generatedID;

    PaperDollItemFrameRec();
    ~PaperDollItemFrameRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

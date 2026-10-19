#ifndef WOW_SOURCE_DB_REC_LOADINGSCREENTAXISPLINESREC_H
#define WOW_SOURCE_DB_REC_LOADINGSCREENTAXISPLINESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class LoadingScreenTaxiSplinesRec {
    public:
    static const int32_t s_numColumns = 19;
    static const int32_t s_rowSize = 76;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_pathID;
    /* 0008 */ float m_locx [8];
    /* 0028 */ float m_locy [8];
    /* 0048 */ int32_t m_legIndex;

    LoadingScreenTaxiSplinesRec();
    ~LoadingScreenTaxiSplinesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

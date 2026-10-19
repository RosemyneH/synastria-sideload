#ifndef WOW_SOURCE_DB_REC_EXHAUSTIONREC_H
#define WOW_SOURCE_DB_REC_EXHAUSTIONREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class ExhaustionRec {
    public:
    static const int32_t s_numColumns = 23;
    static const int32_t s_rowSize = 92;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_xp;
    /* 0008 */ float m_factor;
    /* 000C */ float m_outdoorHours;
    /* 0010 */ float m_innHours;
    /* 0014 */ const char* m_name;
    /* 0018 */ float m_threshold;

    ExhaustionRec();
    ~ExhaustionRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

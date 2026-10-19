#ifndef WOW_SOURCE_DB_REC_ATTACKANIMKITSREC_H
#define WOW_SOURCE_DB_REC_ATTACKANIMKITSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class AttackAnimKitsRec {
    public:
    static const int32_t s_numColumns = 5;
    static const int32_t s_rowSize = 20;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_itemSubclassID;
    /* 0008 */ int32_t m_animTypeID;
    /* 000C */ int32_t m_animFrequency;
    /* 0010 */ int32_t m_whichHand;

    AttackAnimKitsRec();
    ~AttackAnimKitsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

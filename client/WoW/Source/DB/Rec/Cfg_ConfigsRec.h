#ifndef WOW_SOURCE_DB_REC_CFG_CONFIGSREC_H
#define WOW_SOURCE_DB_REC_CFG_CONFIGSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class Cfg_ConfigsRec {
    public:
    static const int32_t s_numColumns = 4;
    static const int32_t s_rowSize = 16;
    static const bool s_generatedID = true;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_realmType;
    /* 0008 */ int32_t m_playerKillingAllowed;
    /* 000C */ int32_t m_roleplaying;
    /* 0010 */ int32_t m_generatedID;

    Cfg_ConfigsRec();
    ~Cfg_ConfigsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

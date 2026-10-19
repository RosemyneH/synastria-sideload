#ifndef WOW_SOURCE_DB_REC_WMOAREATABLEREC_H
#define WOW_SOURCE_DB_REC_WMOAREATABLEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class WMOAreaTableRec {
    public:
    static const int32_t s_numColumns = 28;
    static const int32_t s_rowSize = 112;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_wmoid;
    /* 0008 */ int32_t m_nameSetID;
    /* 000C */ int32_t m_wmogroupID;
    /* 0010 */ int32_t m_soundProviderPref;
    /* 0014 */ int32_t m_soundProviderPrefUnderwater;
    /* 0018 */ int32_t m_ambienceID;
    /* 001C */ int32_t m_zoneMusic;
    /* 0020 */ int32_t m_introSound;
    /* 0024 */ int32_t m_flags;
    /* 0028 */ int32_t m_areaTableID;
    /* 002C */ const char* m_areaName;

    WMOAreaTableRec();
    ~WMOAreaTableRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

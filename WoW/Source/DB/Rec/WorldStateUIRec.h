#ifndef WOW_SOURCE_DB_REC_WORLDSTATEUIREC_H
#define WOW_SOURCE_DB_REC_WORLDSTATEUIREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class WorldStateUIRec {
    public:
    static const int32_t s_numColumns = 63;
    static const int32_t s_rowSize = 252;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_mapID;
    /* 0008 */ int32_t m_areaID;
    /* 000C */ int32_t m_phaseShift;
    /* 0010 */ const char* m_icon;
    /* 0014 */ const char* m_string;
    /* 0018 */ const char* m_tooltip;
    /* 001C */ int32_t m_stateVariable;
    /* 0020 */ int32_t m_type;
    /* 0024 */ const char* m_dynamicIcon;
    /* 0028 */ const char* m_dynamicTooltip;
    /* 002C */ const char* m_extendedUi;
    /* 0030 */ int32_t m_extendedUistateVariable [3];

    WorldStateUIRec();
    ~WorldStateUIRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

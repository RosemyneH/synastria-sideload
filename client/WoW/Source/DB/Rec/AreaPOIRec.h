#ifndef WOW_SOURCE_DB_REC_AREAPOIREC_H
#define WOW_SOURCE_DB_REC_AREAPOIREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class AreaPOIRec {
    public:
    static const int32_t s_numColumns = 54;
    static const int32_t s_rowSize = 216;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_importance;
    /* 0008 */ int32_t m_icon [9];
    /* 002C */ int32_t m_factionID;
    /* 0030 */ float m_pos [3];
    /* 003C */ int32_t m_continentID;
    /* 0040 */ int32_t m_flags;
    /* 0044 */ int32_t m_areaID;
    /* 0048 */ const char* m_name;
    /* 004C */ const char* m_description;
    /* 0050 */ int32_t m_worldStateID;
    /* 0054 */ int32_t m_worldMapLink;

    AreaPOIRec();
    ~AreaPOIRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

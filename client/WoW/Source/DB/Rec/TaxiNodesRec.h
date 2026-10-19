#ifndef WOW_SOURCE_DB_REC_TAXINODESREC_H
#define WOW_SOURCE_DB_REC_TAXINODESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class TaxiNodesRec {
    public:
    static const int32_t s_numColumns = 24;
    static const int32_t s_rowSize = 96;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_continentID;
    /* 0008 */ float m_pos [3];
    /* 0014 */ const char* m_name;
    /* 0018 */ int32_t m_mountCreatureID [2];

    TaxiNodesRec();
    ~TaxiNodesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

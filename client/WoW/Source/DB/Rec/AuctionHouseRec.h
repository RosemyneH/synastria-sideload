#ifndef WOW_SOURCE_DB_REC_AUCTIONHOUSEREC_H
#define WOW_SOURCE_DB_REC_AUCTIONHOUSEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class AuctionHouseRec {
    public:
    static const int32_t s_numColumns = 21;
    static const int32_t s_rowSize = 84;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_factionID;
    /* 0008 */ int32_t m_depositRate;
    /* 000C */ int32_t m_consignmentRate;
    /* 0010 */ const char* m_name;

    AuctionHouseRec();
    ~AuctionHouseRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

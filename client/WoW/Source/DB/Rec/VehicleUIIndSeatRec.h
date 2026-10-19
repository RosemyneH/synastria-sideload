#ifndef WOW_SOURCE_DB_REC_VEHICLEUIINDSEATREC_H
#define WOW_SOURCE_DB_REC_VEHICLEUIINDSEATREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class VehicleUIIndSeatRec {
    public:
    static const int32_t s_numColumns = 5;
    static const int32_t s_rowSize = 20;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_vehicleUiindicatorID;
    /* 0008 */ int32_t m_virtualSeatIndex;
    /* 000C */ float m_xpos;
    /* 0010 */ float m_ypos;

    VehicleUIIndSeatRec();
    ~VehicleUIIndSeatRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

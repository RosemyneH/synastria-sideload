#ifndef WOW_SOURCE_DB_REC_VEHICLESEATREC_H
#define WOW_SOURCE_DB_REC_VEHICLESEATREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class VehicleSeatRec {
    public:
    static const int32_t s_numColumns = 58;
    static const int32_t s_rowSize = 232;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_flags;
    /* 0008 */ int32_t m_attachmentID;
    /* 000C */ float m_attachmentOffset [3];
    /* 0018 */ float m_field33512213004;
    /* 001C */ float m_enterSpeed;
    /* 0020 */ float m_enterGravity;
    /* 0024 */ float m_enterMinDuration;
    /* 0028 */ float m_enterMaxDuration;
    /* 002C */ float m_enterMinArcHeight;
    /* 0030 */ float m_enterMaxArcHeight;
    /* 0034 */ int32_t m_enterAnimStart;
    /* 0038 */ int32_t m_enterAnimLoop;
    /* 003C */ int32_t m_rideAnimStart;
    /* 0040 */ int32_t m_rideAnimLoop;
    /* 0044 */ int32_t m_rideUpperAnimStart;
    /* 0048 */ int32_t m_rideUpperAnimLoop;
    /* 004C */ float m_field33512213017;
    /* 0050 */ float m_exitSpeed;
    /* 0054 */ float m_exitGravity;
    /* 0058 */ float m_exitMinDuration;
    /* 005C */ float m_exitMaxDuration;
    /* 0060 */ float m_exitMinArcHeight;
    /* 0064 */ float m_exitMaxArcHeight;
    /* 0068 */ int32_t m_exitAnimStart;
    /* 006C */ int32_t m_exitAnimLoop;
    /* 0070 */ int32_t m_exitAnimEnd;
    /* 0074 */ float m_field33512213027;
    /* 0078 */ float m_passengerPitch;
    /* 007C */ float m_field33512213029;
    /* 0080 */ int32_t m_passengerAttachmentID;
    /* 0084 */ int32_t m_vehicleEnterAnim;
    /* 0088 */ int32_t m_vehicleExitAnim;
    /* 008C */ int32_t m_vehicleRideAnimLoop;
    /* 0090 */ int32_t m_field33512213034;
    /* 0094 */ int32_t m_vehicleExitAnimBone;
    /* 0098 */ int32_t m_vehicleEnterAnimBone;
    /* 009C */ float m_field33512213037;
    /* 00A0 */ float m_field33512213038;
    /* 00A4 */ int32_t m_vehicleAbilityDisplay;
    /* 00A8 */ int32_t m_enterUisoundID;
    /* 00AC */ int32_t m_field33512213041;
    /* 00B0 */ int32_t m_uiSkin;
    /* 00B4 */ int32_t m_flagsB;
    /* 00B8 */ float m_field33512213044;
    /* 00BC */ float m_field33512213045;
    /* 00C0 */ float m_field33512213046;
    /* 00C4 */ float m_field33512213047;
    /* 00C8 */ float m_field33512213048;
    /* 00CC */ float m_field33512213049;
    /* 00D0 */ float m_field33512213050;
    /* 00D4 */ float m_field33512213051;
    /* 00D8 */ float m_field33512213052;
    /* 00DC */ float m_field33512213053;
    /* 00E0 */ float m_field33512213054;
    /* 00E4 */ float m_field33512213055;

    VehicleSeatRec();
    ~VehicleSeatRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

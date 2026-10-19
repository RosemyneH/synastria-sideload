#ifndef WOW_SOURCE_DB_REC_VEHICLEREC_H
#define WOW_SOURCE_DB_REC_VEHICLEREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class VehicleRec {
    public:
    static const int32_t s_numColumns = 40;
    static const int32_t s_rowSize = 160;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_flags;
    /* 0008 */ float m_turnSpeed;
    /* 000C */ float m_pitchSpeed;
    /* 0010 */ float m_pitchMin;
    /* 0014 */ float m_pitchMax;
    /* 0018 */ int32_t m_seatID [8];
    /* 0038 */ float m_mouseLookOffsetPitch;
    /* 003C */ float m_cameraFadeDistScalarMin;
    /* 0040 */ float m_cameraFadeDistScalarMax;
    /* 0044 */ float m_cameraPitchOffset;
    /* 0048 */ float m_facingLimitRight;
    /* 004C */ float m_facingLimitLeft;
    /* 0050 */ float m_msslTrgtTurnLingering;
    /* 0054 */ float m_msslTrgtPitchLingering;
    /* 0058 */ float m_msslTrgtMouseLingering;
    /* 005C */ float m_msslTrgtEndOpacity;
    /* 0060 */ float m_msslTrgtArcSpeed;
    /* 0064 */ float m_msslTrgtArcRepeat;
    /* 0068 */ float m_msslTrgtArcWidth;
    /* 006C */ float m_msslTrgtImpactRadius [2];
    /* 0074 */ const char* m_msslTrgtArcTexture;
    /* 0078 */ const char* m_msslTrgtImpactTexture;
    /* 007C */ const char* m_msslTrgtImpactModel [2];
    /* 0084 */ float m_cameraYawOffset;
    /* 0088 */ int32_t m_uiLocomotionType;
    /* 008C */ float m_msslTrgtImpactTexRadius;
    /* 0090 */ int32_t m_vehicleUiindicatorID;
    /* 0094 */ int32_t m_powerDisplayID [3];

    VehicleRec();
    ~VehicleRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

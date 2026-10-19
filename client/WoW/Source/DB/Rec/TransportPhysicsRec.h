#ifndef WOW_SOURCE_DB_REC_TRANSPORTPHYSICSREC_H
#define WOW_SOURCE_DB_REC_TRANSPORTPHYSICSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class TransportPhysicsRec {
    public:
    static const int32_t s_numColumns = 11;
    static const int32_t s_rowSize = 44;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ float m_waveAmp;
    /* 0008 */ float m_waveTimeScale;
    /* 000C */ float m_rollAmp;
    /* 0010 */ float m_rollTimeScale;
    /* 0014 */ float m_pitchAmp;
    /* 0018 */ float m_pitchTimeScale;
    /* 001C */ float m_maxBank;
    /* 0020 */ float m_maxBankTurnSpeed;
    /* 0024 */ float m_speedDampThresh;
    /* 0028 */ float m_speedDamp;

    TransportPhysicsRec();
    ~TransportPhysicsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

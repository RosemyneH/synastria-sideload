#include "WoW/Source/DB/Rec/TransportPhysicsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

TransportPhysicsRec::TransportPhysicsRec() {
}

TransportPhysicsRec::~TransportPhysicsRec() {
}

// VERIFY: 1cc8aa91 auto
// OFFSET: 0x008BA5A0
const char* TransportPhysicsRec::GetFilename() {
    return "DBFilesClient\\TransportPhysics.dbc";
}

bool TransportPhysicsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t waveAmpRead = SFileReadFile(file, &this->m_waveAmp, sizeof(this->m_waveAmp), nullptr, nullptr, 0);
    int32_t waveTimeScaleRead = SFileReadFile(file, &this->m_waveTimeScale, sizeof(this->m_waveTimeScale), nullptr, nullptr, 0);
    int32_t rollAmpRead = SFileReadFile(file, &this->m_rollAmp, sizeof(this->m_rollAmp), nullptr, nullptr, 0);
    int32_t rollTimeScaleRead = SFileReadFile(file, &this->m_rollTimeScale, sizeof(this->m_rollTimeScale), nullptr, nullptr, 0);
    int32_t pitchAmpRead = SFileReadFile(file, &this->m_pitchAmp, sizeof(this->m_pitchAmp), nullptr, nullptr, 0);
    int32_t pitchTimeScaleRead = SFileReadFile(file, &this->m_pitchTimeScale, sizeof(this->m_pitchTimeScale), nullptr, nullptr, 0);
    int32_t maxBankRead = SFileReadFile(file, &this->m_maxBank, sizeof(this->m_maxBank), nullptr, nullptr, 0);
    int32_t maxBankTurnSpeedRead = SFileReadFile(file, &this->m_maxBankTurnSpeed, sizeof(this->m_maxBankTurnSpeed), nullptr, nullptr, 0);
    int32_t speedDampThreshRead = SFileReadFile(file, &this->m_speedDampThresh, sizeof(this->m_speedDampThresh), nullptr, nullptr, 0);
    int32_t speedDampRead = SFileReadFile(file, &this->m_speedDamp, sizeof(this->m_speedDamp), nullptr, nullptr, 0);

    if (!idRead
        | !waveAmpRead
        | !waveTimeScaleRead
        | !rollAmpRead
        | !rollTimeScaleRead
        | !pitchAmpRead
        | !pitchTimeScaleRead
        | !maxBankRead
        | !maxBankTurnSpeedRead
        | !speedDampThreshRead
        | !speedDampRead) {
        return false;
    }

    return true;
}

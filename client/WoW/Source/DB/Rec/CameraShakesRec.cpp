#include "WoW/Source/DB/Rec/CameraShakesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CameraShakesRec::CameraShakesRec() {
}

CameraShakesRec::~CameraShakesRec() {
}

// VERIFY: fa1845b9 auto
// OFFSET: 0x008AAA90
const char* CameraShakesRec::GetFilename() {
    return "DBFilesClient\\CameraShakes.dbc";
}

// VERIFY: 6380c6e7 manual
// OFFSET: 0x008BCFA0
bool CameraShakesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t shakeTypeRead = SFileReadFile(file, &this->m_shakeType, sizeof(this->m_shakeType), nullptr, nullptr, 0);
    int32_t directionRead = SFileReadFile(file, &this->m_direction, sizeof(this->m_direction), nullptr, nullptr, 0);
    int32_t amplitudeRead = SFileReadFile(file, &this->m_amplitude, sizeof(this->m_amplitude), nullptr, nullptr, 0);
    int32_t frequencyRead = SFileReadFile(file, &this->m_frequency, sizeof(this->m_frequency), nullptr, nullptr, 0);
    int32_t durationRead = SFileReadFile(file, &this->m_duration, sizeof(this->m_duration), nullptr, nullptr, 0);
    int32_t phaseRead = SFileReadFile(file, &this->m_phase, sizeof(this->m_phase), nullptr, nullptr, 0);
    int32_t coefficientRead = SFileReadFile(file, &this->m_coefficient, sizeof(this->m_coefficient), nullptr, nullptr, 0);

    if (!idRead
        | !shakeTypeRead
        | !directionRead
        | !amplitudeRead
        | !frequencyRead
        | !durationRead
        | !phaseRead
        | !coefficientRead) {
        return false;
    }

    return true;
}

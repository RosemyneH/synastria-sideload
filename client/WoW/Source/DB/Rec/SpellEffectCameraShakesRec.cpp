#include "WoW/Source/DB/Rec/SpellEffectCameraShakesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellEffectCameraShakesRec::SpellEffectCameraShakesRec() {
}

SpellEffectCameraShakesRec::~SpellEffectCameraShakesRec() {
}

// VERIFY: 722bd6dc auto
// OFFSET: 0x008B7D90
const char* SpellEffectCameraShakesRec::GetFilename() {
    return "DBFilesClient\\SpellEffectCameraShakes.dbc";
}

// VERIFY: 3a2078f5 auto
// OFFSET: 0x008BA750
bool SpellEffectCameraShakesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t cameraShakeRead = SFileReadFile(file, this->m_cameraShake, sizeof(this->m_cameraShake), nullptr, nullptr, 0);

    if (!idRead
        | !cameraShakeRead) {
        return false;
    }

    return true;
}

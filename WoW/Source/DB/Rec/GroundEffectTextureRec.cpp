#include "WoW/Source/DB/Rec/GroundEffectTextureRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

GroundEffectTextureRec::GroundEffectTextureRec() {
}

GroundEffectTextureRec::~GroundEffectTextureRec() {
}

// VERIFY: dbf854a7 auto
// OFFSET: 0x008AFEC0
const char* GroundEffectTextureRec::GetFilename() {
    return "DBFilesClient\\GroundEffectTexture.dbc";
}

bool GroundEffectTextureRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t doodadIDRead = SFileReadFile(file, this->m_doodadID, sizeof(this->m_doodadID), nullptr, nullptr, 0);
    int32_t doodadWeightRead = SFileReadFile(file, this->m_doodadWeight, sizeof(this->m_doodadWeight), nullptr, nullptr, 0);
    int32_t densityRead = SFileReadFile(file, &this->m_density, sizeof(this->m_density), nullptr, nullptr, 0);
    int32_t soundRead = SFileReadFile(file, &this->m_sound, sizeof(this->m_sound), nullptr, nullptr, 0);

    if (!idRead
        | !doodadIDRead
        | !doodadWeightRead
        | !densityRead
        | !soundRead) {
        return false;
    }

    return true;
}

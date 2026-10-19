#include "WoW/Source/DB/Rec/ParticleColorRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

ParticleColorRec::ParticleColorRec() {
}

ParticleColorRec::~ParticleColorRec() {
}

// VERIFY: e0998434 auto
// OFFSET: 0x008B4D70
const char* ParticleColorRec::GetFilename() {
    return "DBFilesClient\\ParticleColor.dbc";
}

// VERIFY: 28bc09aa manual
// OFFSET: 0x008B4D80
bool ParticleColorRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t startRead = SFileReadFile(file, this->m_start, sizeof(this->m_start), nullptr, nullptr, 0);
    int32_t midRead = SFileReadFile(file, this->m_mid, sizeof(this->m_mid), nullptr, nullptr, 0);
    int32_t endRead = SFileReadFile(file, this->m_end, sizeof(this->m_end), nullptr, nullptr, 0);

    if (!idRead
        | !startRead
        | !midRead
        | !endRead) {
        return false;
    }

    return true;
}

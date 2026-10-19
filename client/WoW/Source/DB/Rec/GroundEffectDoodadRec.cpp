#include "WoW/Source/DB/Rec/GroundEffectDoodadRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

GroundEffectDoodadRec::GroundEffectDoodadRec() {
}

GroundEffectDoodadRec::~GroundEffectDoodadRec() {
}

// VERIFY: 8ed0d168 auto
// OFFSET: 0x008AFEB0
const char* GroundEffectDoodadRec::GetFilename() {
    return "DBFilesClient\\GroundEffectDoodad.dbc";
}

bool GroundEffectDoodadRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t doodadpathOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t doodadpathRead = SFileReadFile(file, &doodadpathOffset, sizeof(doodadpathOffset), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);

    if (!idRead
        | !doodadpathRead
        | !flagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_doodadpath = &stringBuffer[doodadpathOffset];
    } else {
        this->m_doodadpath = "";
    }

    return true;
}

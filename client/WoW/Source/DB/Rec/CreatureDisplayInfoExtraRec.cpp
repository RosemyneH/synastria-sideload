#include "WoW/Source/DB/Rec/CreatureDisplayInfoExtraRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CreatureDisplayInfoExtraRec::CreatureDisplayInfoExtraRec() {
}

CreatureDisplayInfoExtraRec::~CreatureDisplayInfoExtraRec() {
}

// VERIFY: 9078a98f auto
// OFFSET: 0x008AD010
const char* CreatureDisplayInfoExtraRec::GetFilename() {
    return "DBFilesClient\\CreatureDisplayInfoExtra.dbc";
}

// VERIFY: f714f2d2 auto
// OFFSET: 0x008AD020
bool CreatureDisplayInfoExtraRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t bakeNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t displayRaceIDRead = SFileReadFile(file, &this->m_displayRaceID, sizeof(this->m_displayRaceID), nullptr, nullptr, 0);
    int32_t displaySexIDRead = SFileReadFile(file, &this->m_displaySexID, sizeof(this->m_displaySexID), nullptr, nullptr, 0);
    int32_t skinIDRead = SFileReadFile(file, &this->m_skinID, sizeof(this->m_skinID), nullptr, nullptr, 0);
    int32_t faceIDRead = SFileReadFile(file, &this->m_faceID, sizeof(this->m_faceID), nullptr, nullptr, 0);
    int32_t hairStyleIDRead = SFileReadFile(file, &this->m_hairStyleID, sizeof(this->m_hairStyleID), nullptr, nullptr, 0);
    int32_t hairColorIDRead = SFileReadFile(file, &this->m_hairColorID, sizeof(this->m_hairColorID), nullptr, nullptr, 0);
    int32_t facialHairIDRead = SFileReadFile(file, &this->m_facialHairID, sizeof(this->m_facialHairID), nullptr, nullptr, 0);
    int32_t npcitemDisplayRead = SFileReadFile(file, this->m_npcitemDisplay, sizeof(this->m_npcitemDisplay), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t bakeNameRead = SFileReadFile(file, &bakeNameOffset, sizeof(bakeNameOffset), nullptr, nullptr, 0);

    if (!idRead
        | !displayRaceIDRead
        | !displaySexIDRead
        | !skinIDRead
        | !faceIDRead
        | !hairStyleIDRead
        | !hairColorIDRead
        | !facialHairIDRead
        | !npcitemDisplayRead
        | !flagsRead
        | !bakeNameRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_bakeName = &stringBuffer[bakeNameOffset];
    } else {
        this->m_bakeName = "";
    }

    return true;
}

#include "WoW/Source/DB/Rec/SoundEmittersRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SoundEmittersRec::SoundEmittersRec() {
}

SoundEmittersRec::~SoundEmittersRec() {
}

// VERIFY: 079de937 auto
// OFFSET: 0x008B6820
const char* SoundEmittersRec::GetFilename() {
    return "DBFilesClient\\SoundEmitters.dbc";
}

// VERIFY: db701d78 auto
// OFFSET: 0x008B6830
bool SoundEmittersRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t positionRead0 = SFileReadFile(file, &this->m_position[0], sizeof(this->m_position[0]), nullptr, nullptr, 0);
    int32_t positionRead1 = SFileReadFile(file, &this->m_position[1], sizeof(this->m_position[1]), nullptr, nullptr, 0);
    int32_t positionRead2 = SFileReadFile(file, &this->m_position[2], sizeof(this->m_position[2]), nullptr, nullptr, 0);
    int32_t directionRead0 = SFileReadFile(file, &this->m_direction[0], sizeof(this->m_direction[0]), nullptr, nullptr, 0);
    int32_t directionRead1 = SFileReadFile(file, &this->m_direction[1], sizeof(this->m_direction[1]), nullptr, nullptr, 0);
    int32_t directionRead2 = SFileReadFile(file, &this->m_direction[2], sizeof(this->m_direction[2]), nullptr, nullptr, 0);
    int32_t soundEntryAdvancedIDRead = SFileReadFile(file, &this->m_soundEntryAdvancedID, sizeof(this->m_soundEntryAdvancedID), nullptr, nullptr, 0);
    int32_t mapIDRead = SFileReadFile(file, &this->m_mapID, sizeof(this->m_mapID), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);

    if (!idRead
        | !positionRead0
        | !positionRead1
        | !positionRead2
        | !directionRead0
        | !directionRead1
        | !directionRead2
        | !soundEntryAdvancedIDRead
        | !mapIDRead
        | !nameRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
    } else {
        this->m_name = "";
    }

    return true;
}

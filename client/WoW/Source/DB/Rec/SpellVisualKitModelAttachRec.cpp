#include "WoW/Source/DB/Rec/SpellVisualKitModelAttachRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellVisualKitModelAttachRec::SpellVisualKitModelAttachRec() {
}

SpellVisualKitModelAttachRec::~SpellVisualKitModelAttachRec() {
}

// VERIFY: 57269c2f auto
// OFFSET: 0x008B93D0
const char* SpellVisualKitModelAttachRec::GetFilename() {
    return "DBFilesClient\\SpellVisualKitModelAttach.dbc";
}

bool SpellVisualKitModelAttachRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t parentSpellVisualKitIDRead = SFileReadFile(file, &this->m_parentSpellVisualKitID, sizeof(this->m_parentSpellVisualKitID), nullptr, nullptr, 0);
    int32_t spellVisualEffectNameIDRead = SFileReadFile(file, &this->m_spellVisualEffectNameID, sizeof(this->m_spellVisualEffectNameID), nullptr, nullptr, 0);
    int32_t attachmentIDRead = SFileReadFile(file, &this->m_attachmentID, sizeof(this->m_attachmentID), nullptr, nullptr, 0);
    int32_t offsetRead0 = SFileReadFile(file, &this->m_offset[0], sizeof(this->m_offset[0]), nullptr, nullptr, 0);
    int32_t offsetRead1 = SFileReadFile(file, &this->m_offset[1], sizeof(this->m_offset[1]), nullptr, nullptr, 0);
    int32_t offsetRead2 = SFileReadFile(file, &this->m_offset[2], sizeof(this->m_offset[2]), nullptr, nullptr, 0);
    int32_t yawRead = SFileReadFile(file, &this->m_yaw, sizeof(this->m_yaw), nullptr, nullptr, 0);
    int32_t pitchRead = SFileReadFile(file, &this->m_pitch, sizeof(this->m_pitch), nullptr, nullptr, 0);
    int32_t rollRead = SFileReadFile(file, &this->m_roll, sizeof(this->m_roll), nullptr, nullptr, 0);

    if (!idRead
        | !parentSpellVisualKitIDRead
        | !spellVisualEffectNameIDRead
        | !attachmentIDRead
        | !offsetRead0
        | !offsetRead1
        | !offsetRead2
        | !yawRead
        | !pitchRead
        | !rollRead) {
        return false;
    }

    return true;
}

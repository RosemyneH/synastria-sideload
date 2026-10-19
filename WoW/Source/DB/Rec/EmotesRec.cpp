#include "WoW/Source/DB/Rec/EmotesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

EmotesRec::EmotesRec() {
}

EmotesRec::~EmotesRec() {
}

// VERIFY: 56e2bcb9 auto
// OFFSET: 0x008AEDC0
const char* EmotesRec::GetFilename() {
    return "DBFilesClient\\Emotes.dbc";
}

bool EmotesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t emoteSlashCommandOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t emoteSlashCommandRead = SFileReadFile(file, &emoteSlashCommandOffset, sizeof(emoteSlashCommandOffset), nullptr, nullptr, 0);
    int32_t animIDRead = SFileReadFile(file, &this->m_animID, sizeof(this->m_animID), nullptr, nullptr, 0);
    int32_t emoteFlagsRead = SFileReadFile(file, &this->m_emoteFlags, sizeof(this->m_emoteFlags), nullptr, nullptr, 0);
    int32_t emoteSpecProcRead = SFileReadFile(file, &this->m_emoteSpecProc, sizeof(this->m_emoteSpecProc), nullptr, nullptr, 0);
    int32_t emoteSpecProcParamRead = SFileReadFile(file, &this->m_emoteSpecProcParam, sizeof(this->m_emoteSpecProcParam), nullptr, nullptr, 0);
    int32_t eventSoundIDRead = SFileReadFile(file, &this->m_eventSoundID, sizeof(this->m_eventSoundID), nullptr, nullptr, 0);

    if (!idRead
        | !emoteSlashCommandRead
        | !animIDRead
        | !emoteFlagsRead
        | !emoteSpecProcRead
        | !emoteSpecProcParamRead
        | !eventSoundIDRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_emoteSlashCommand = &stringBuffer[emoteSlashCommandOffset];
    } else {
        this->m_emoteSlashCommand = "";
    }

    return true;
}

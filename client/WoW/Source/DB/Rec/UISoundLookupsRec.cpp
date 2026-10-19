#include "WoW/Source/DB/Rec/UISoundLookupsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

UISoundLookupsRec::UISoundLookupsRec() {
}

UISoundLookupsRec::~UISoundLookupsRec() {
}

// VERIFY: daa61280 auto
// OFFSET: 0x008BA730
const char* UISoundLookupsRec::GetFilename() {
    return "DBFilesClient\\UISoundLookups.dbc";
}

bool UISoundLookupsRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t soundNameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);
    int32_t soundNameRead = SFileReadFile(file, &soundNameOffset, sizeof(soundNameOffset), nullptr, nullptr, 0);

    if (!idRead
        | !soundIDRead
        | !soundNameRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_soundName = &stringBuffer[soundNameOffset];
    } else {
        this->m_soundName = "";
    }

    return true;
}

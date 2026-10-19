#include "WoW/Source/DB/Rec/StringLookupsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

StringLookupsRec::StringLookupsRec() {
}

StringLookupsRec::~StringLookupsRec() {
}

// VERIFY: 4d309d1b auto
// OFFSET: 0x008B98C0
const char* StringLookupsRec::GetFilename() {
    return "DBFilesClient\\StringLookups.dbc";
}

bool StringLookupsRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t stringOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t stringRead = SFileReadFile(file, &stringOffset, sizeof(stringOffset), nullptr, nullptr, 0);

    if (!idRead
        | !stringRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_string = &stringBuffer[stringOffset];
    } else {
        this->m_string = "";
    }

    return true;
}

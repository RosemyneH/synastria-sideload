#include "WoW/Source/DB/Rec/UnitBloodLevelsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

UnitBloodLevelsRec::UnitBloodLevelsRec() {
}

UnitBloodLevelsRec::~UnitBloodLevelsRec() {
}

// VERIFY: 22cd7205 auto
// OFFSET: 0x008BA740
const char* UnitBloodLevelsRec::GetFilename() {
    return "DBFilesClient\\UnitBloodLevels.dbc";
}

bool UnitBloodLevelsRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t violencelevelRead = SFileReadFile(file, this->m_violencelevel, sizeof(this->m_violencelevel), nullptr, nullptr, 0);

    if (!idRead
        | !violencelevelRead) {
        return false;
    }

    return true;
}

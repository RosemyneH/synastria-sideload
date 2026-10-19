#include "WoW/Source/DB/Rec/GameTablesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

GameTablesRec::GameTablesRec() {
}

GameTablesRec::~GameTablesRec() {
}

// VERIFY: 596c7158 auto
// OFFSET: 0x008AFD20
const char* GameTablesRec::GetFilename() {
    return "DBFilesClient\\GameTables.dbc";
}

// VERIFY: a7a21b5f auto
// OFFSET: 0x008AFD30
bool GameTablesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;

    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t numRowsRead = SFileReadFile(file, &this->m_numRows, sizeof(this->m_numRows), nullptr, nullptr, 0);
    int32_t numColumnsRead = SFileReadFile(file, &this->m_numColumns, sizeof(this->m_numColumns), nullptr, nullptr, 0);

    if (!nameRead
        | !numRowsRead
        | !numColumnsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
    } else {
        this->m_name = "";
    }

    return true;
}

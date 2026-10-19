#include "WoW/Source/DB/Rec/SpellMissileMotionRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellMissileMotionRec::SpellMissileMotionRec() {
}

SpellMissileMotionRec::~SpellMissileMotionRec() {
}

// VERIFY: 1146fd6c auto
// OFFSET: 0x008B84F0
const char* SpellMissileMotionRec::GetFilename() {
    return "DBFilesClient\\SpellMissileMotion.dbc";
}

// VERIFY: 957e3870 auto
// OFFSET: 0x008B8500
bool SpellMissileMotionRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;
    uint32_t scriptBodyOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t scriptBodyRead = SFileReadFile(file, &scriptBodyOffset, sizeof(scriptBodyOffset), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t missileCountRead = SFileReadFile(file, &this->m_missileCount, sizeof(this->m_missileCount), nullptr, nullptr, 0);

    if (!idRead
        | !nameRead
        | !scriptBodyRead
        | !flagsRead
        | !missileCountRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
        this->m_scriptBody = &stringBuffer[scriptBodyOffset];
    } else {
        this->m_name = "";
        this->m_scriptBody = "";
    }

    return true;
}

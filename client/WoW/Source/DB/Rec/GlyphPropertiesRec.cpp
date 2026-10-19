#include "WoW/Source/DB/Rec/GlyphPropertiesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

GlyphPropertiesRec::GlyphPropertiesRec() {
}

GlyphPropertiesRec::~GlyphPropertiesRec() {
}

// VERIFY: 5aa8bb62 auto
// OFFSET: 0x008AFDF0
const char* GlyphPropertiesRec::GetFilename() {
    return "DBFilesClient\\GlyphProperties.dbc";
}

bool GlyphPropertiesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t spellIDRead = SFileReadFile(file, &this->m_spellID, sizeof(this->m_spellID), nullptr, nullptr, 0);
    int32_t glyphSlotFlagsRead = SFileReadFile(file, &this->m_glyphSlotFlags, sizeof(this->m_glyphSlotFlags), nullptr, nullptr, 0);
    int32_t spellIconIDRead = SFileReadFile(file, &this->m_spellIconID, sizeof(this->m_spellIconID), nullptr, nullptr, 0);

    if (!idRead
        | !spellIDRead
        | !glyphSlotFlagsRead
        | !spellIconIDRead) {
        return false;
    }

    return true;
}

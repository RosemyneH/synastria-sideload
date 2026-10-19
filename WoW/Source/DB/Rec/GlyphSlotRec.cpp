#include "WoW/Source/DB/Rec/GlyphSlotRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

GlyphSlotRec::GlyphSlotRec() {
}

GlyphSlotRec::~GlyphSlotRec() {
}

// VERIFY: 1c374d19 auto
// OFFSET: 0x008AFE00
const char* GlyphSlotRec::GetFilename() {
    return "DBFilesClient\\GlyphSlot.dbc";
}

bool GlyphSlotRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t typeRead = SFileReadFile(file, &this->m_type, sizeof(this->m_type), nullptr, nullptr, 0);
    int32_t tooltipRead = SFileReadFile(file, &this->m_tooltip, sizeof(this->m_tooltip), nullptr, nullptr, 0);

    if (!idRead
        | !typeRead
        | !tooltipRead) {
        return false;
    }

    return true;
}

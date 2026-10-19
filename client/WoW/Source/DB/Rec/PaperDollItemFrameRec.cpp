#include "WoW/Source/DB/Rec/PaperDollItemFrameRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

PaperDollItemFrameRec::PaperDollItemFrameRec() {
}

PaperDollItemFrameRec::~PaperDollItemFrameRec() {
}

// VERIFY: 1696320d auto
// OFFSET: 0x008B4CB0
const char* PaperDollItemFrameRec::GetFilename() {
    return "DBFilesClient\\PaperDollItemFrame.dbc";
}

// VERIFY: 64d49e0c auto
// OFFSET: 0x008B4CC0
bool PaperDollItemFrameRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t itemButtonNameOffset;
    uint32_t slotIconOffset;

    int32_t itemButtonNameRead = SFileReadFile(file, &itemButtonNameOffset, sizeof(itemButtonNameOffset), nullptr, nullptr, 0);
    int32_t slotIconRead = SFileReadFile(file, &slotIconOffset, sizeof(slotIconOffset), nullptr, nullptr, 0);
    int32_t slotNumberRead = SFileReadFile(file, &this->m_slotNumber, sizeof(this->m_slotNumber), nullptr, nullptr, 0);

    if (!itemButtonNameRead
        | !slotIconRead
        | !slotNumberRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_itemButtonName = &stringBuffer[itemButtonNameOffset];
        this->m_slotIcon = &stringBuffer[slotIconOffset];
    } else {
        this->m_itemButtonName = "";
        this->m_slotIcon = "";
    }

    return true;
}

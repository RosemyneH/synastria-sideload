#include "WoW/Source/DB/Rec/GMTicketCategoryRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

GMTicketCategoryRec::GMTicketCategoryRec() {
}

GMTicketCategoryRec::~GMTicketCategoryRec() {
}

// VERIFY: cf165486 auto
// OFFSET: 0x008AFEA0
const char* GMTicketCategoryRec::GetFilename() {
    return "DBFilesClient\\GMTicketCategory.dbc";
}

bool GMTicketCategoryRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t categoryOffset[16];
    uint32_t categoryFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t categoryRead0 = SFileReadFile(file, &categoryOffset[0], sizeof(categoryOffset[0]), nullptr, nullptr, 0);
    int32_t categoryRead1 = SFileReadFile(file, &categoryOffset[1], sizeof(categoryOffset[1]), nullptr, nullptr, 0);
    int32_t categoryRead2 = SFileReadFile(file, &categoryOffset[2], sizeof(categoryOffset[2]), nullptr, nullptr, 0);
    int32_t categoryRead3 = SFileReadFile(file, &categoryOffset[3], sizeof(categoryOffset[3]), nullptr, nullptr, 0);
    int32_t categoryRead4 = SFileReadFile(file, &categoryOffset[4], sizeof(categoryOffset[4]), nullptr, nullptr, 0);
    int32_t categoryRead5 = SFileReadFile(file, &categoryOffset[5], sizeof(categoryOffset[5]), nullptr, nullptr, 0);
    int32_t categoryRead6 = SFileReadFile(file, &categoryOffset[6], sizeof(categoryOffset[6]), nullptr, nullptr, 0);
    int32_t categoryRead7 = SFileReadFile(file, &categoryOffset[7], sizeof(categoryOffset[7]), nullptr, nullptr, 0);
    int32_t categoryRead8 = SFileReadFile(file, &categoryOffset[8], sizeof(categoryOffset[8]), nullptr, nullptr, 0);
    int32_t categoryRead9 = SFileReadFile(file, &categoryOffset[9], sizeof(categoryOffset[9]), nullptr, nullptr, 0);
    int32_t categoryRead10 = SFileReadFile(file, &categoryOffset[10], sizeof(categoryOffset[10]), nullptr, nullptr, 0);
    int32_t categoryRead11 = SFileReadFile(file, &categoryOffset[11], sizeof(categoryOffset[11]), nullptr, nullptr, 0);
    int32_t categoryRead12 = SFileReadFile(file, &categoryOffset[12], sizeof(categoryOffset[12]), nullptr, nullptr, 0);
    int32_t categoryRead13 = SFileReadFile(file, &categoryOffset[13], sizeof(categoryOffset[13]), nullptr, nullptr, 0);
    int32_t categoryRead14 = SFileReadFile(file, &categoryOffset[14], sizeof(categoryOffset[14]), nullptr, nullptr, 0);
    int32_t categoryRead15 = SFileReadFile(file, &categoryOffset[15], sizeof(categoryOffset[15]), nullptr, nullptr, 0);
    int32_t categoryFlagsRead = SFileReadFile(file, &categoryFlags, sizeof(categoryFlags), nullptr, nullptr, 0);

    if (!idRead
        | !categoryRead0
        | !categoryRead1
        | !categoryRead2
        | !categoryRead3
        | !categoryRead4
        | !categoryRead5
        | !categoryRead6
        | !categoryRead7
        | !categoryRead8
        | !categoryRead9
        | !categoryRead10
        | !categoryRead11
        | !categoryRead12
        | !categoryRead13
        | !categoryRead14
        | !categoryRead15
        | !categoryFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_category = &stringBuffer[categoryOffset[s_locale]];
    } else {
        this->m_category = "";
    }

    return true;
}

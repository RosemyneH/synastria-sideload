#include "WoW/Source/DB/Rec/LoadingScreenTaxiSplinesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

LoadingScreenTaxiSplinesRec::LoadingScreenTaxiSplinesRec() {
}

LoadingScreenTaxiSplinesRec::~LoadingScreenTaxiSplinesRec() {
}

// VERIFY: cabcf3fb auto
// OFFSET: 0x008B2BF0
const char* LoadingScreenTaxiSplinesRec::GetFilename() {
    return "DBFilesClient\\LoadingScreenTaxiSplines.dbc";
}

// VERIFY: 19351ca8 manual
// OFFSET: 0x008B2C00
bool LoadingScreenTaxiSplinesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t pathIDRead = SFileReadFile(file, &this->m_pathID, sizeof(this->m_pathID), nullptr, nullptr, 0);
    int32_t locxRead = SFileReadFile(file, this->m_locx, sizeof(this->m_locx), nullptr, nullptr, 0);
    int32_t locyRead = SFileReadFile(file, this->m_locy, sizeof(this->m_locy), nullptr, nullptr, 0);
    int32_t legIndexRead = SFileReadFile(file, &this->m_legIndex, sizeof(this->m_legIndex), nullptr, nullptr, 0);

    if (!idRead
        | !pathIDRead
        | !locxRead
        | !locyRead
        | !legIndexRead) {
        return false;
    }

    return true;
}

#include "WoW/Source/DB/Rec/TaxiPathRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

TaxiPathRec::TaxiPathRec() {
}

TaxiPathRec::~TaxiPathRec() {
}

// VERIFY: ace04574 auto
// OFFSET: 0x008BA180
const char* TaxiPathRec::GetFilename() {
    return "DBFilesClient\\TaxiPath.dbc";
}

bool TaxiPathRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t fromTaxiNodeRead = SFileReadFile(file, &this->m_fromTaxiNode, sizeof(this->m_fromTaxiNode), nullptr, nullptr, 0);
    int32_t toTaxiNodeRead = SFileReadFile(file, &this->m_toTaxiNode, sizeof(this->m_toTaxiNode), nullptr, nullptr, 0);
    int32_t costRead = SFileReadFile(file, &this->m_cost, sizeof(this->m_cost), nullptr, nullptr, 0);

    if (!idRead
        | !fromTaxiNodeRead
        | !toTaxiNodeRead
        | !costRead) {
        return false;
    }

    return true;
}

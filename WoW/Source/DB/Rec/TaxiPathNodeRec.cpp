#include "WoW/Source/DB/Rec/TaxiPathNodeRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

TaxiPathNodeRec::TaxiPathNodeRec() {
}

TaxiPathNodeRec::~TaxiPathNodeRec() {
}

// VERIFY: 78f6f62f auto
// OFFSET: 0x008BA170
const char* TaxiPathNodeRec::GetFilename() {
    return "DBFilesClient\\TaxiPathNode.dbc";
}

// VERIFY: 7dbb2de8 auto
// OFFSET: 0x008BA5B0
bool TaxiPathNodeRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t pathIDRead = SFileReadFile(file, &this->m_pathID, sizeof(this->m_pathID), nullptr, nullptr, 0);
    int32_t nodeIndexRead = SFileReadFile(file, &this->m_nodeIndex, sizeof(this->m_nodeIndex), nullptr, nullptr, 0);
    int32_t continentIDRead = SFileReadFile(file, &this->m_continentID, sizeof(this->m_continentID), nullptr, nullptr, 0);
    int32_t locRead0 = SFileReadFile(file, &this->m_loc[0], sizeof(this->m_loc[0]), nullptr, nullptr, 0);
    int32_t locRead1 = SFileReadFile(file, &this->m_loc[1], sizeof(this->m_loc[1]), nullptr, nullptr, 0);
    int32_t locRead2 = SFileReadFile(file, &this->m_loc[2], sizeof(this->m_loc[2]), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t delayRead = SFileReadFile(file, &this->m_delay, sizeof(this->m_delay), nullptr, nullptr, 0);
    int32_t arrivalEventIDRead = SFileReadFile(file, &this->m_arrivalEventID, sizeof(this->m_arrivalEventID), nullptr, nullptr, 0);
    int32_t departureEventIDRead = SFileReadFile(file, &this->m_departureEventID, sizeof(this->m_departureEventID), nullptr, nullptr, 0);

    if (!idRead
        | !pathIDRead
        | !nodeIndexRead
        | !continentIDRead
        | !locRead0
        | !locRead1
        | !locRead2
        | !flagsRead
        | !delayRead
        | !arrivalEventIDRead
        | !departureEventIDRead) {
        return false;
    }

    return true;
}

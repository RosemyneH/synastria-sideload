#include "WoW/Source/DB/Rec/CinematicCameraRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CinematicCameraRec::CinematicCameraRec() {
}

CinematicCameraRec::~CinematicCameraRec() {
}

// VERIFY: dd2eea75 auto
// OFFSET: 0x008ACE60
const char* CinematicCameraRec::GetFilename() {
    return "DBFilesClient\\CinematicCamera.dbc";
}

// VERIFY: 1270de90 manual
// OFFSET: 0x008ACE70
bool CinematicCameraRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t modelOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t modelRead = SFileReadFile(file, &modelOffset, sizeof(modelOffset), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);
    int32_t originRead0 = SFileReadFile(file, &this->m_origin[0], sizeof(this->m_origin[0]), nullptr, nullptr, 0);
    int32_t originRead1 = SFileReadFile(file, &this->m_origin[1], sizeof(this->m_origin[1]), nullptr, nullptr, 0);
    int32_t originRead2 = SFileReadFile(file, &this->m_origin[2], sizeof(this->m_origin[2]), nullptr, nullptr, 0);
    int32_t originFacingRead = SFileReadFile(file, &this->m_originFacing, sizeof(this->m_originFacing), nullptr, nullptr, 0);

    if (!idRead
        | !modelRead
        | !soundIDRead
        | !originRead0
        | !originRead1
        | !originRead2
        | !originFacingRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_model = &stringBuffer[modelOffset];
    } else {
        this->m_model = "";
    }

    return true;
}

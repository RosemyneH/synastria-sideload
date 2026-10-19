#include "WoW/Source/DB/Rec/CinematicSequencesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

CinematicSequencesRec::CinematicSequencesRec() {
}

CinematicSequencesRec::~CinematicSequencesRec() {
}

// VERIFY: c63c6f48 auto
// OFFSET: 0x008ACF90
const char* CinematicSequencesRec::GetFilename() {
    return "DBFilesClient\\CinematicSequences.dbc";
}

// VERIFY: abaf627c manual
// OFFSET: 0x008ACFA0
bool CinematicSequencesRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t soundIDRead = SFileReadFile(file, &this->m_soundID, sizeof(this->m_soundID), nullptr, nullptr, 0);
    int32_t cameraRead = SFileReadFile(file, this->m_camera, sizeof(this->m_camera), nullptr, nullptr, 0);

    if (!idRead
        | !soundIDRead
        | !cameraRead) {
        return false;
    }

    return true;
}

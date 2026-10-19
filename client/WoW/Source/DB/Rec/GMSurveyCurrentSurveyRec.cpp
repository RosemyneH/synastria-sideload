#include "WoW/Source/DB/Rec/GMSurveyCurrentSurveyRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

GMSurveyCurrentSurveyRec::GMSurveyCurrentSurveyRec() {
}

GMSurveyCurrentSurveyRec::~GMSurveyCurrentSurveyRec() {
}

// VERIFY: 18363f31 auto
// OFFSET: 0x008AFE20
const char* GMSurveyCurrentSurveyRec::GetFilename() {
    return "DBFilesClient\\GMSurveyCurrentSurvey.dbc";
}

bool GMSurveyCurrentSurveyRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t gmsurveyIDRead = SFileReadFile(file, &this->m_gmsurveyID, sizeof(this->m_gmsurveyID), nullptr, nullptr, 0);

    if (!idRead
        | !gmsurveyIDRead) {
        return false;
    }

    return true;
}

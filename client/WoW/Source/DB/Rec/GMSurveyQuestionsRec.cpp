#include "WoW/Source/DB/Rec/GMSurveyQuestionsRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

GMSurveyQuestionsRec::GMSurveyQuestionsRec() {
}

GMSurveyQuestionsRec::~GMSurveyQuestionsRec() {
}

// VERIFY: bc0f5d86 auto
// OFFSET: 0x008AFE30
const char* GMSurveyQuestionsRec::GetFilename() {
    return "DBFilesClient\\GMSurveyQuestions.dbc";
}

bool GMSurveyQuestionsRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t questionOffset[16];
    uint32_t questionFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t questionRead0 = SFileReadFile(file, &questionOffset[0], sizeof(questionOffset[0]), nullptr, nullptr, 0);
    int32_t questionRead1 = SFileReadFile(file, &questionOffset[1], sizeof(questionOffset[1]), nullptr, nullptr, 0);
    int32_t questionRead2 = SFileReadFile(file, &questionOffset[2], sizeof(questionOffset[2]), nullptr, nullptr, 0);
    int32_t questionRead3 = SFileReadFile(file, &questionOffset[3], sizeof(questionOffset[3]), nullptr, nullptr, 0);
    int32_t questionRead4 = SFileReadFile(file, &questionOffset[4], sizeof(questionOffset[4]), nullptr, nullptr, 0);
    int32_t questionRead5 = SFileReadFile(file, &questionOffset[5], sizeof(questionOffset[5]), nullptr, nullptr, 0);
    int32_t questionRead6 = SFileReadFile(file, &questionOffset[6], sizeof(questionOffset[6]), nullptr, nullptr, 0);
    int32_t questionRead7 = SFileReadFile(file, &questionOffset[7], sizeof(questionOffset[7]), nullptr, nullptr, 0);
    int32_t questionRead8 = SFileReadFile(file, &questionOffset[8], sizeof(questionOffset[8]), nullptr, nullptr, 0);
    int32_t questionRead9 = SFileReadFile(file, &questionOffset[9], sizeof(questionOffset[9]), nullptr, nullptr, 0);
    int32_t questionRead10 = SFileReadFile(file, &questionOffset[10], sizeof(questionOffset[10]), nullptr, nullptr, 0);
    int32_t questionRead11 = SFileReadFile(file, &questionOffset[11], sizeof(questionOffset[11]), nullptr, nullptr, 0);
    int32_t questionRead12 = SFileReadFile(file, &questionOffset[12], sizeof(questionOffset[12]), nullptr, nullptr, 0);
    int32_t questionRead13 = SFileReadFile(file, &questionOffset[13], sizeof(questionOffset[13]), nullptr, nullptr, 0);
    int32_t questionRead14 = SFileReadFile(file, &questionOffset[14], sizeof(questionOffset[14]), nullptr, nullptr, 0);
    int32_t questionRead15 = SFileReadFile(file, &questionOffset[15], sizeof(questionOffset[15]), nullptr, nullptr, 0);
    int32_t questionFlagsRead = SFileReadFile(file, &questionFlags, sizeof(questionFlags), nullptr, nullptr, 0);

    if (!idRead
        | !questionRead0
        | !questionRead1
        | !questionRead2
        | !questionRead3
        | !questionRead4
        | !questionRead5
        | !questionRead6
        | !questionRead7
        | !questionRead8
        | !questionRead9
        | !questionRead10
        | !questionRead11
        | !questionRead12
        | !questionRead13
        | !questionRead14
        | !questionRead15
        | !questionFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_question = &stringBuffer[questionOffset[s_locale]];
    } else {
        this->m_question = "";
    }

    return true;
}

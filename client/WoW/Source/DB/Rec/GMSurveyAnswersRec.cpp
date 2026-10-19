#include "WoW/Source/DB/Rec/GMSurveyAnswersRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

GMSurveyAnswersRec::GMSurveyAnswersRec() {
}

GMSurveyAnswersRec::~GMSurveyAnswersRec() {
}

// VERIFY: c0f15259 auto
// OFFSET: 0x008AFE10
const char* GMSurveyAnswersRec::GetFilename() {
    return "DBFilesClient\\GMSurveyAnswers.dbc";
}

// VERIFY: d8d8a0a5 auto
// OFFSET: 0x008B53C0
bool GMSurveyAnswersRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t answerOffset[16];
    uint32_t answerFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t sortIndexRead = SFileReadFile(file, &this->m_sortIndex, sizeof(this->m_sortIndex), nullptr, nullptr, 0);
    int32_t gmsurveyQuestionIDRead = SFileReadFile(file, &this->m_gmsurveyQuestionID, sizeof(this->m_gmsurveyQuestionID), nullptr, nullptr, 0);
    int32_t answerRead0 = SFileReadFile(file, &answerOffset[0], sizeof(answerOffset[0]), nullptr, nullptr, 0);
    int32_t answerRead1 = SFileReadFile(file, &answerOffset[1], sizeof(answerOffset[1]), nullptr, nullptr, 0);
    int32_t answerRead2 = SFileReadFile(file, &answerOffset[2], sizeof(answerOffset[2]), nullptr, nullptr, 0);
    int32_t answerRead3 = SFileReadFile(file, &answerOffset[3], sizeof(answerOffset[3]), nullptr, nullptr, 0);
    int32_t answerRead4 = SFileReadFile(file, &answerOffset[4], sizeof(answerOffset[4]), nullptr, nullptr, 0);
    int32_t answerRead5 = SFileReadFile(file, &answerOffset[5], sizeof(answerOffset[5]), nullptr, nullptr, 0);
    int32_t answerRead6 = SFileReadFile(file, &answerOffset[6], sizeof(answerOffset[6]), nullptr, nullptr, 0);
    int32_t answerRead7 = SFileReadFile(file, &answerOffset[7], sizeof(answerOffset[7]), nullptr, nullptr, 0);
    int32_t answerRead8 = SFileReadFile(file, &answerOffset[8], sizeof(answerOffset[8]), nullptr, nullptr, 0);
    int32_t answerRead9 = SFileReadFile(file, &answerOffset[9], sizeof(answerOffset[9]), nullptr, nullptr, 0);
    int32_t answerRead10 = SFileReadFile(file, &answerOffset[10], sizeof(answerOffset[10]), nullptr, nullptr, 0);
    int32_t answerRead11 = SFileReadFile(file, &answerOffset[11], sizeof(answerOffset[11]), nullptr, nullptr, 0);
    int32_t answerRead12 = SFileReadFile(file, &answerOffset[12], sizeof(answerOffset[12]), nullptr, nullptr, 0);
    int32_t answerRead13 = SFileReadFile(file, &answerOffset[13], sizeof(answerOffset[13]), nullptr, nullptr, 0);
    int32_t answerRead14 = SFileReadFile(file, &answerOffset[14], sizeof(answerOffset[14]), nullptr, nullptr, 0);
    int32_t answerRead15 = SFileReadFile(file, &answerOffset[15], sizeof(answerOffset[15]), nullptr, nullptr, 0);
    int32_t answerFlagsRead = SFileReadFile(file, &answerFlags, sizeof(answerFlags), nullptr, nullptr, 0);

    if (!idRead
        | !sortIndexRead
        | !gmsurveyQuestionIDRead
        | !answerRead0
        | !answerRead1
        | !answerRead2
        | !answerRead3
        | !answerRead4
        | !answerRead5
        | !answerRead6
        | !answerRead7
        | !answerRead8
        | !answerRead9
        | !answerRead10
        | !answerRead11
        | !answerRead12
        | !answerRead13
        | !answerRead14
        | !answerRead15
        | !answerFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_answer = &stringBuffer[answerOffset[s_locale]];
    } else {
        this->m_answer = "";
    }

    return true;
}

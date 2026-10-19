#ifndef WOW_SOURCE_UI_GMSURVEYINFO_H
#define WOW_SOURCE_UI_GMSURVEYINFO_H

#include <cstddef>
#include <cstdint>

class CGGMSurveyInfo {
    public:
    static uint8_t m_surveyAnswers[10];
    static char* m_surveyAnswerComments[10];
    static char* m_surveyComment;
    static void ShutdownGame();
    static void FreeAnswers();
    static void InitializeGame();
    static void SetAnswer(uint32_t index, uint8_t answer, const char* comment);
    static void SetComment(const char* comment);
    static const char* GetQuestion(uint32_t index);
    static const char* GetAnswer(uint32_t index, int32_t answer);
    static int32_t GetNumAnswers(uint32_t index);
    static void Submit();
};

#endif

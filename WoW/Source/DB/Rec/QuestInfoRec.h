#ifndef WOW_SOURCE_DB_REC_QUESTINFOREC_H
#define WOW_SOURCE_DB_REC_QUESTINFOREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class QuestInfoRec {
    public:
    static const int32_t s_numColumns = 18;
    static const int32_t s_rowSize = 72;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_infoName;

    QuestInfoRec();
    ~QuestInfoRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

#ifndef WOW_SOURCE_DB_REC_EMOTESREC_H
#define WOW_SOURCE_DB_REC_EMOTESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class EmotesRec {
    public:
    static const int32_t s_numColumns = 7;
    static const int32_t s_rowSize = 28;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_emoteSlashCommand;
    /* 0008 */ int32_t m_animID;
    /* 000C */ int32_t m_emoteFlags;
    /* 0010 */ int32_t m_emoteSpecProc;
    /* 0014 */ int32_t m_emoteSpecProcParam;
    /* 0018 */ int32_t m_eventSoundID;

    EmotesRec();
    ~EmotesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

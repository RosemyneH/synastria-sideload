#ifndef WOW_SOURCE_DB_REC_CINEMATICSEQUENCESREC_H
#define WOW_SOURCE_DB_REC_CINEMATICSEQUENCESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class CinematicSequencesRec {
    public:
    static const int32_t s_numColumns = 10;
    static const int32_t s_rowSize = 40;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_soundID;
    /* 0008 */ int32_t m_camera [8];

    CinematicSequencesRec();
    ~CinematicSequencesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

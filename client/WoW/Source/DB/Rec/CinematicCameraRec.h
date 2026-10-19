#ifndef WOW_SOURCE_DB_REC_CINEMATICCAMERAREC_H
#define WOW_SOURCE_DB_REC_CINEMATICCAMERAREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class CinematicCameraRec {
    public:
    static const int32_t s_numColumns = 7;
    static const int32_t s_rowSize = 28;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_model;
    /* 0008 */ int32_t m_soundID;
    /* 000C */ float m_origin [3];
    /* 0018 */ float m_originFacing;

    CinematicCameraRec();
    ~CinematicCameraRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

#ifndef WOW_SOURCE_DB_REC_GROUNDEFFECTTEXTUREREC_H
#define WOW_SOURCE_DB_REC_GROUNDEFFECTTEXTUREREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class GroundEffectTextureRec {
    public:
    static const int32_t s_numColumns = 11;
    static const int32_t s_rowSize = 44;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_doodadID [4];
    /* 0014 */ int32_t m_doodadWeight [4];
    /* 0024 */ int32_t m_density;
    /* 0028 */ int32_t m_sound;

    GroundEffectTextureRec();
    ~GroundEffectTextureRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

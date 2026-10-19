#ifndef WOW_SOURCE_DB_REC_GROUNDEFFECTDOODADREC_H
#define WOW_SOURCE_DB_REC_GROUNDEFFECTDOODADREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class GroundEffectDoodadRec {
    public:
    static const int32_t s_numColumns = 3;
    static const int32_t s_rowSize = 12;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_doodadpath;
    /* 0008 */ int32_t m_flags;

    GroundEffectDoodadRec();
    ~GroundEffectDoodadRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

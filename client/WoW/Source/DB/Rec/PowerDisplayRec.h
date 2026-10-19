#ifndef WOW_SOURCE_DB_REC_POWERDISPLAYREC_H
#define WOW_SOURCE_DB_REC_POWERDISPLAYREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class PowerDisplayRec {
    public:
    static const int32_t s_numColumns = 6;
    static const int32_t s_rowSize = 15;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_actualType;
    /* 0008 */ const char* m_globalStringBaseTag;
    /* 000C */ int8_t m_red;
    /* 000D */ int8_t m_green;
    /* 000E */ int8_t m_blue;

    PowerDisplayRec();
    ~PowerDisplayRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

#ifndef WOW_SOURCE_DB_REC_SPELLFOCUSOBJECTREC_H
#define WOW_SOURCE_DB_REC_SPELLFOCUSOBJECTREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SpellFocusObjectRec {
    public:
    static const int32_t s_numColumns = 18;
    static const int32_t s_rowSize = 72;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_name;

    SpellFocusObjectRec();
    ~SpellFocusObjectRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

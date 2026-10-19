#ifndef WOW_SOURCE_DB_REC_SHEATHESOUNDLOOKUPSREC_H
#define WOW_SOURCE_DB_REC_SHEATHESOUNDLOOKUPSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SheatheSoundLookupsRec {
    public:
    static const int32_t s_numColumns = 7;
    static const int32_t s_rowSize = 28;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_classID;
    /* 0008 */ int32_t m_subclassID;
    /* 000C */ int32_t m_material;
    /* 0010 */ int32_t m_checkMaterial;
    /* 0014 */ int32_t m_sheatheSound;
    /* 0018 */ int32_t m_unsheatheSound;

    SheatheSoundLookupsRec();
    ~SheatheSoundLookupsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

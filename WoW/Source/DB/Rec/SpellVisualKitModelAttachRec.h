#ifndef WOW_SOURCE_DB_REC_SPELLVISUALKITMODELATTACHREC_H
#define WOW_SOURCE_DB_REC_SPELLVISUALKITMODELATTACHREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SpellVisualKitModelAttachRec {
    public:
    static const int32_t s_numColumns = 10;
    static const int32_t s_rowSize = 40;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_parentSpellVisualKitID;
    /* 0008 */ int32_t m_spellVisualEffectNameID;
    /* 000C */ int32_t m_attachmentID;
    /* 0010 */ float m_offset [3];
    /* 001C */ float m_yaw;
    /* 0020 */ float m_pitch;
    /* 0024 */ float m_roll;

    SpellVisualKitModelAttachRec();
    ~SpellVisualKitModelAttachRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

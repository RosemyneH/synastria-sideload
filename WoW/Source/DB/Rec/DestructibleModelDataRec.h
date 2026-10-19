#ifndef WOW_SOURCE_DB_REC_DESTRUCTIBLEMODELDATAREC_H
#define WOW_SOURCE_DB_REC_DESTRUCTIBLEMODELDATAREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class DestructibleModelDataRec {
    public:
    static const int32_t s_numColumns = 19;
    static const int32_t s_rowSize = 76;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_state0ImpactEffectDoodadSet;
    /* 0008 */ int32_t m_state0AmbientDoodadSet;
    /* 000C */ int32_t m_state1WMO;
    /* 0010 */ int32_t m_state1DestructionDoodadSet;
    /* 0014 */ int32_t m_state1ImpactEffectDoodadSet;
    /* 0018 */ int32_t m_state1AmbientDoodadSet;
    /* 001C */ int32_t m_state2WMO;
    /* 0020 */ int32_t m_state2DestructionDoodadSet;
    /* 0024 */ int32_t m_state2ImpactEffectDoodadSet;
    /* 0028 */ int32_t m_state2AmbientDoodadSet;
    /* 002C */ int32_t m_state3WMO;
    /* 0030 */ int32_t m_state3InitDoodadSet;
    /* 0034 */ int32_t m_state3AmbientDoodadSet;
    /* 0038 */ int32_t m_ejectDirection;
    /* 003C */ int32_t m_repairGroundFx;
    /* 0040 */ int32_t m_doNotHighlight;
    /* 0044 */ int32_t m_healEffect;
    /* 0048 */ int32_t m_healEffectSpeed;

    DestructibleModelDataRec();
    ~DestructibleModelDataRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

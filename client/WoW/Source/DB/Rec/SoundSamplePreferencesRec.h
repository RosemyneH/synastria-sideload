#ifndef WOW_SOURCE_DB_REC_SOUNDSAMPLEPREFERENCESREC_H
#define WOW_SOURCE_DB_REC_SOUNDSAMPLEPREFERENCESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SoundSamplePreferencesRec {
    public:
    static const int32_t s_numColumns = 17;
    static const int32_t s_rowSize = 68;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_field0603592001;
    /* 0008 */ int32_t m_field0603592002;
    /* 000C */ int32_t m_eax2SampleRoom;
    /* 0010 */ int32_t m_field0603592004;
    /* 0014 */ int32_t m_field0603592005;
    /* 0018 */ float m_field0603592006;
    /* 001C */ int32_t m_field0603592007;
    /* 0020 */ float m_eax2SampleOcclusionLfratio;
    /* 0024 */ float m_eax2SampleOcclusionRoomRatio;
    /* 0028 */ int32_t m_field0603592010;
    /* 002C */ float m_eax1EffectLevel;
    /* 0030 */ int32_t m_field0603592012;
    /* 0034 */ float m_field0603592013;
    /* 0038 */ float m_eax3SampleExclusion;
    /* 003C */ float m_field0603592015;
    /* 0040 */ int32_t m_field0603592016;

    SoundSamplePreferencesRec();
    ~SoundSamplePreferencesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

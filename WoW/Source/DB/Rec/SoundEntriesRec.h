#ifndef WOW_SOURCE_DB_REC_SOUNDENTRIESREC_H
#define WOW_SOURCE_DB_REC_SOUNDENTRIESREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class SoundEntriesRec {
    public:
    static const int32_t s_numColumns = 30;
    static const int32_t s_rowSize = 120;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_soundType;
    /* 0008 */ const char* m_name;
    /* 000C */ const char* m_file [10];
    /* 0034 */ int32_t m_freq [10];
    /* 005C */ const char* m_directoryBase;
    /* 0060 */ float m_volumeFloat;
    /* 0064 */ int32_t m_flags;
    /* 0068 */ float m_minDistance;
    /* 006C */ float m_distanceCutoff;
    /* 0070 */ int32_t m_eaxdef;
    /* 0074 */ int32_t m_soundEntriesAdvancedID;

    SoundEntriesRec();
    ~SoundEntriesRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

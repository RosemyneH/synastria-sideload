#ifndef WOW_SOURCE_DB_REC_CHATCHANNELSREC_H
#define WOW_SOURCE_DB_REC_CHATCHANNELSREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class ChatChannelsRec {
    public:
    static const int32_t s_numColumns = 37;
    static const int32_t s_rowSize = 148;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_flags;
    /* 0008 */ int32_t m_factionGroup;
    /* 000C */ const char* m_name;
    /* 0010 */ const char* m_shortcut;

    ChatChannelsRec();
    ~ChatChannelsRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

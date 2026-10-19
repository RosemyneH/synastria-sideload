#ifndef WOW_SOURCE_DB_REC_ACHIEVEMENTREC_H
#define WOW_SOURCE_DB_REC_ACHIEVEMENTREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class AchievementRec {
    public:
    static const int32_t s_numColumns = 62;
    static const int32_t s_rowSize = 248;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ int32_t m_faction;
    /* 0008 */ int32_t m_instanceID;
    /* 000C */ int32_t m_supercedes;
    /* 0010 */ const char* m_title;
    /* 0014 */ const char* m_description;
    /* 0018 */ int32_t m_category;
    /* 001C */ int32_t m_points;
    /* 0020 */ int32_t m_uiOrder;
    /* 0024 */ int32_t m_flags;
    /* 0028 */ int32_t m_iconID;
    /* 002C */ const char* m_reward;
    /* 0030 */ int32_t m_minimumCriteria;
    /* 0034 */ int32_t m_sharesCriteria;

    AchievementRec();
    ~AchievementRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

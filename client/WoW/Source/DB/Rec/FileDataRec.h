#ifndef WOW_SOURCE_DB_REC_FILEDATAREC_H
#define WOW_SOURCE_DB_REC_FILEDATAREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class FileDataRec {
    public:
    static const int32_t s_numColumns = 3;
    static const int32_t s_rowSize = 12;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_filename;
    /* 0008 */ const char* m_filepath;

    FileDataRec();
    ~FileDataRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

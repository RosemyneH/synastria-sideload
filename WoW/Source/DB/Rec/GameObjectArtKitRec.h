#ifndef WOW_SOURCE_DB_REC_GAMEOBJECTARTKITREC_H
#define WOW_SOURCE_DB_REC_GAMEOBJECTARTKITREC_H

#include <cstddef>
#include <cstdint>

class SFileImpl;

class GameObjectArtKitRec {
    public:
    static const int32_t s_numColumns = 8;
    static const int32_t s_rowSize = 32;
    static const bool s_generatedID = false;

    /* 0000 */ int32_t m_id;
    /* 0004 */ const char* m_textureVariation [3];
    /* 0010 */ const char* m_attachModel [4];

    GameObjectArtKitRec();
    ~GameObjectArtKitRec();
    static const char* GetFilename();
    bool Read(SFileImpl* file, const char* stringBuffer);
};

#endif

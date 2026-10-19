#include "WoW/Source/DB/Rec/WeatherRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

WeatherRec::WeatherRec() {
}

WeatherRec::~WeatherRec() {
}

// VERIFY: 9a849373 auto
// OFFSET: 0x008BB6F0
const char* WeatherRec::GetFilename() {
    return "DBFilesClient\\Weather.dbc";
}

// VERIFY: fea22a4f manual
// OFFSET: 0x008BB700
bool WeatherRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t effectTextureOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t ambienceIDRead = SFileReadFile(file, &this->m_ambienceID, sizeof(this->m_ambienceID), nullptr, nullptr, 0);
    int32_t effectTypeRead = SFileReadFile(file, &this->m_effectType, sizeof(this->m_effectType), nullptr, nullptr, 0);
    int32_t transitionSkyBoxRead = SFileReadFile(file, &this->m_transitionSkyBox, sizeof(this->m_transitionSkyBox), nullptr, nullptr, 0);
    int32_t effectColorRead = SFileReadFile(file, this->m_effectColor, sizeof(this->m_effectColor), nullptr, nullptr, 0);
    int32_t effectTextureRead = SFileReadFile(file, &effectTextureOffset, sizeof(effectTextureOffset), nullptr, nullptr, 0);

    if (!idRead
        | !ambienceIDRead
        | !effectTypeRead
        | !transitionSkyBoxRead
        | !effectColorRead
        | !effectTextureRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_effectTexture = &stringBuffer[effectTextureOffset];
    } else {
        this->m_effectTexture = "";
    }

    return true;
}

#include "WoW/Source/DB/Rec/SoundProviderPreferencesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SoundProviderPreferencesRec::SoundProviderPreferencesRec() {
}

SoundProviderPreferencesRec::~SoundProviderPreferencesRec() {
}

// VERIFY: 1ecdcb6f auto
// OFFSET: 0x008B6D10
const char* SoundProviderPreferencesRec::GetFilename() {
    return "DBFilesClient\\SoundProviderPreferences.dbc";
}

// VERIFY: 56d29a82 auto
// OFFSET: 0x008B6D20
bool SoundProviderPreferencesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t descriptionOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t descriptionRead = SFileReadFile(file, &descriptionOffset, sizeof(descriptionOffset), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t eaxenvironmentSelectionRead = SFileReadFile(file, &this->m_eaxenvironmentSelection, sizeof(this->m_eaxenvironmentSelection), nullptr, nullptr, 0);
    int32_t eaxdecayTimeRead = SFileReadFile(file, &this->m_eaxdecayTime, sizeof(this->m_eaxdecayTime), nullptr, nullptr, 0);
    int32_t eax2EnvironmentSizeRead = SFileReadFile(file, &this->m_eax2EnvironmentSize, sizeof(this->m_eax2EnvironmentSize), nullptr, nullptr, 0);
    int32_t eax2EnvironmentDiffusionRead = SFileReadFile(file, &this->m_eax2EnvironmentDiffusion, sizeof(this->m_eax2EnvironmentDiffusion), nullptr, nullptr, 0);
    int32_t eax2RoomRead = SFileReadFile(file, &this->m_eax2Room, sizeof(this->m_eax2Room), nullptr, nullptr, 0);
    int32_t eax2RoomHfRead = SFileReadFile(file, &this->m_eax2RoomHf, sizeof(this->m_eax2RoomHf), nullptr, nullptr, 0);
    int32_t eax2DecayHfratioRead = SFileReadFile(file, &this->m_eax2DecayHfratio, sizeof(this->m_eax2DecayHfratio), nullptr, nullptr, 0);
    int32_t eax2ReflectionsRead = SFileReadFile(file, &this->m_eax2Reflections, sizeof(this->m_eax2Reflections), nullptr, nullptr, 0);
    int32_t eax2ReflectionsDelayRead = SFileReadFile(file, &this->m_eax2ReflectionsDelay, sizeof(this->m_eax2ReflectionsDelay), nullptr, nullptr, 0);
    int32_t eax2ReverbRead = SFileReadFile(file, &this->m_eax2Reverb, sizeof(this->m_eax2Reverb), nullptr, nullptr, 0);
    int32_t eax2ReverbDelayRead = SFileReadFile(file, &this->m_eax2ReverbDelay, sizeof(this->m_eax2ReverbDelay), nullptr, nullptr, 0);
    int32_t eax2RoomRolloffRead = SFileReadFile(file, &this->m_eax2RoomRolloff, sizeof(this->m_eax2RoomRolloff), nullptr, nullptr, 0);
    int32_t eax2AirAbsorptionRead = SFileReadFile(file, &this->m_eax2AirAbsorption, sizeof(this->m_eax2AirAbsorption), nullptr, nullptr, 0);
    int32_t eax3RoomLfRead = SFileReadFile(file, &this->m_eax3RoomLf, sizeof(this->m_eax3RoomLf), nullptr, nullptr, 0);
    int32_t eax3DecayLfratioRead = SFileReadFile(file, &this->m_eax3DecayLfratio, sizeof(this->m_eax3DecayLfratio), nullptr, nullptr, 0);
    int32_t eax3EchoTimeRead = SFileReadFile(file, &this->m_eax3EchoTime, sizeof(this->m_eax3EchoTime), nullptr, nullptr, 0);
    int32_t eax3EchoDepthRead = SFileReadFile(file, &this->m_eax3EchoDepth, sizeof(this->m_eax3EchoDepth), nullptr, nullptr, 0);
    int32_t eax3ModulationTimeRead = SFileReadFile(file, &this->m_eax3ModulationTime, sizeof(this->m_eax3ModulationTime), nullptr, nullptr, 0);
    int32_t eax3ModulationDepthRead = SFileReadFile(file, &this->m_eax3ModulationDepth, sizeof(this->m_eax3ModulationDepth), nullptr, nullptr, 0);
    int32_t eax3HfreferenceRead = SFileReadFile(file, &this->m_eax3Hfreference, sizeof(this->m_eax3Hfreference), nullptr, nullptr, 0);
    int32_t eax3LfreferenceRead = SFileReadFile(file, &this->m_eax3Lfreference, sizeof(this->m_eax3Lfreference), nullptr, nullptr, 0);

    if (!idRead
        | !descriptionRead
        | !flagsRead
        | !eaxenvironmentSelectionRead
        | !eaxdecayTimeRead
        | !eax2EnvironmentSizeRead
        | !eax2EnvironmentDiffusionRead
        | !eax2RoomRead
        | !eax2RoomHfRead
        | !eax2DecayHfratioRead
        | !eax2ReflectionsRead
        | !eax2ReflectionsDelayRead
        | !eax2ReverbRead
        | !eax2ReverbDelayRead
        | !eax2RoomRolloffRead
        | !eax2AirAbsorptionRead
        | !eax3RoomLfRead
        | !eax3DecayLfratioRead
        | !eax3EchoTimeRead
        | !eax3EchoDepthRead
        | !eax3ModulationTimeRead
        | !eax3ModulationDepthRead
        | !eax3HfreferenceRead
        | !eax3LfreferenceRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_description = &stringBuffer[descriptionOffset];
    } else {
        this->m_description = "";
    }

    return true;
}

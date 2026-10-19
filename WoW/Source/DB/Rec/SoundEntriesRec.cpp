#include "WoW/Source/DB/Rec/SoundEntriesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SoundEntriesRec::SoundEntriesRec() {
}

SoundEntriesRec::~SoundEntriesRec() {
}

// VERIFY: 97bb2a0e auto
// OFFSET: 0x008B69B0
const char* SoundEntriesRec::GetFilename() {
    return "DBFilesClient\\SoundEntries.dbc";
}

// VERIFY: a420f032 auto
// OFFSET: 0x008B69C0
bool SoundEntriesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset;
    uint32_t fileOffset[10];
    uint32_t directoryBaseOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t soundTypeRead = SFileReadFile(file, &this->m_soundType, sizeof(this->m_soundType), nullptr, nullptr, 0);
    int32_t nameRead = SFileReadFile(file, &nameOffset, sizeof(nameOffset), nullptr, nullptr, 0);
    int32_t fileRead0 = SFileReadFile(file, &fileOffset[0], sizeof(fileOffset[0]), nullptr, nullptr, 0);
    int32_t fileRead1 = SFileReadFile(file, &fileOffset[1], sizeof(fileOffset[1]), nullptr, nullptr, 0);
    int32_t fileRead2 = SFileReadFile(file, &fileOffset[2], sizeof(fileOffset[2]), nullptr, nullptr, 0);
    int32_t fileRead3 = SFileReadFile(file, &fileOffset[3], sizeof(fileOffset[3]), nullptr, nullptr, 0);
    int32_t fileRead4 = SFileReadFile(file, &fileOffset[4], sizeof(fileOffset[4]), nullptr, nullptr, 0);
    int32_t fileRead5 = SFileReadFile(file, &fileOffset[5], sizeof(fileOffset[5]), nullptr, nullptr, 0);
    int32_t fileRead6 = SFileReadFile(file, &fileOffset[6], sizeof(fileOffset[6]), nullptr, nullptr, 0);
    int32_t fileRead7 = SFileReadFile(file, &fileOffset[7], sizeof(fileOffset[7]), nullptr, nullptr, 0);
    int32_t fileRead8 = SFileReadFile(file, &fileOffset[8], sizeof(fileOffset[8]), nullptr, nullptr, 0);
    int32_t fileRead9 = SFileReadFile(file, &fileOffset[9], sizeof(fileOffset[9]), nullptr, nullptr, 0);
    int32_t freqRead = SFileReadFile(file, this->m_freq, sizeof(this->m_freq), nullptr, nullptr, 0);
    int32_t directoryBaseRead = SFileReadFile(file, &directoryBaseOffset, sizeof(directoryBaseOffset), nullptr, nullptr, 0);
    int32_t volumeFloatRead = SFileReadFile(file, &this->m_volumeFloat, sizeof(this->m_volumeFloat), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t minDistanceRead = SFileReadFile(file, &this->m_minDistance, sizeof(this->m_minDistance), nullptr, nullptr, 0);
    int32_t distanceCutoffRead = SFileReadFile(file, &this->m_distanceCutoff, sizeof(this->m_distanceCutoff), nullptr, nullptr, 0);
    int32_t eaxdefRead = SFileReadFile(file, &this->m_eaxdef, sizeof(this->m_eaxdef), nullptr, nullptr, 0);
    int32_t soundEntriesAdvancedIDRead = SFileReadFile(file, &this->m_soundEntriesAdvancedID, sizeof(this->m_soundEntriesAdvancedID), nullptr, nullptr, 0);

    if (!idRead
        | !soundTypeRead
        | !nameRead
        | !fileRead0
        | !fileRead1
        | !fileRead2
        | !fileRead3
        | !fileRead4
        | !fileRead5
        | !fileRead6
        | !fileRead7
        | !fileRead8
        | !fileRead9
        | !freqRead
        | !directoryBaseRead
        | !volumeFloatRead
        | !flagsRead
        | !minDistanceRead
        | !distanceCutoffRead
        | !eaxdefRead
        | !soundEntriesAdvancedIDRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset];
        this->m_file[0] = &stringBuffer[fileOffset[0]];
        this->m_file[1] = &stringBuffer[fileOffset[1]];
        this->m_file[2] = &stringBuffer[fileOffset[2]];
        this->m_file[3] = &stringBuffer[fileOffset[3]];
        this->m_file[4] = &stringBuffer[fileOffset[4]];
        this->m_file[5] = &stringBuffer[fileOffset[5]];
        this->m_file[6] = &stringBuffer[fileOffset[6]];
        this->m_file[7] = &stringBuffer[fileOffset[7]];
        this->m_file[8] = &stringBuffer[fileOffset[8]];
        this->m_file[9] = &stringBuffer[fileOffset[9]];
        this->m_directoryBase = &stringBuffer[directoryBaseOffset];
    } else {
        this->m_name = "";
        this->m_file[0] = "";
        this->m_file[1] = "";
        this->m_file[2] = "";
        this->m_file[3] = "";
        this->m_file[4] = "";
        this->m_file[5] = "";
        this->m_file[6] = "";
        this->m_file[7] = "";
        this->m_file[8] = "";
        this->m_file[9] = "";
        this->m_directoryBase = "";
    }

    return true;
}

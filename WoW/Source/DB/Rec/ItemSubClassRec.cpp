#include "WoW/Source/DB/Rec/ItemSubClassRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

ItemSubClassRec::ItemSubClassRec() {
}

ItemSubClassRec::~ItemSubClassRec() {
}

// VERIFY: dbf5b208 auto
// OFFSET: 0x008B12D0
const char* ItemSubClassRec::GetFilename() {
    return "DBFilesClient\\ItemSubClass.dbc";
}

// VERIFY: 988cfb78 auto
// OFFSET: 0x008B12E0
bool ItemSubClassRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t displayNameOffset[16];
    uint32_t displayNameFlags;
    uint32_t verboseNameOffset[16];
    uint32_t verboseNameFlags;

    int32_t classIDRead = SFileReadFile(file, &this->m_classID, sizeof(this->m_classID), nullptr, nullptr, 0);
    int32_t subClassIDRead = SFileReadFile(file, &this->m_subClassID, sizeof(this->m_subClassID), nullptr, nullptr, 0);
    int32_t prerequisiteProficiencyRead = SFileReadFile(file, &this->m_prerequisiteProficiency, sizeof(this->m_prerequisiteProficiency), nullptr, nullptr, 0);
    int32_t postrequisiteProficiencyRead = SFileReadFile(file, &this->m_postrequisiteProficiency, sizeof(this->m_postrequisiteProficiency), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t displayFlagsRead = SFileReadFile(file, &this->m_displayFlags, sizeof(this->m_displayFlags), nullptr, nullptr, 0);
    int32_t weaponParrySeqRead = SFileReadFile(file, &this->m_weaponParrySeq, sizeof(this->m_weaponParrySeq), nullptr, nullptr, 0);
    int32_t weaponReadySeqRead = SFileReadFile(file, &this->m_weaponReadySeq, sizeof(this->m_weaponReadySeq), nullptr, nullptr, 0);
    int32_t weaponAttackSeqRead = SFileReadFile(file, &this->m_weaponAttackSeq, sizeof(this->m_weaponAttackSeq), nullptr, nullptr, 0);
    int32_t weaponSwingSizeRead = SFileReadFile(file, &this->m_weaponSwingSize, sizeof(this->m_weaponSwingSize), nullptr, nullptr, 0);
    int32_t displayNameRead0 = SFileReadFile(file, &displayNameOffset[0], sizeof(displayNameOffset[0]), nullptr, nullptr, 0);
    int32_t displayNameRead1 = SFileReadFile(file, &displayNameOffset[1], sizeof(displayNameOffset[1]), nullptr, nullptr, 0);
    int32_t displayNameRead2 = SFileReadFile(file, &displayNameOffset[2], sizeof(displayNameOffset[2]), nullptr, nullptr, 0);
    int32_t displayNameRead3 = SFileReadFile(file, &displayNameOffset[3], sizeof(displayNameOffset[3]), nullptr, nullptr, 0);
    int32_t displayNameRead4 = SFileReadFile(file, &displayNameOffset[4], sizeof(displayNameOffset[4]), nullptr, nullptr, 0);
    int32_t displayNameRead5 = SFileReadFile(file, &displayNameOffset[5], sizeof(displayNameOffset[5]), nullptr, nullptr, 0);
    int32_t displayNameRead6 = SFileReadFile(file, &displayNameOffset[6], sizeof(displayNameOffset[6]), nullptr, nullptr, 0);
    int32_t displayNameRead7 = SFileReadFile(file, &displayNameOffset[7], sizeof(displayNameOffset[7]), nullptr, nullptr, 0);
    int32_t displayNameRead8 = SFileReadFile(file, &displayNameOffset[8], sizeof(displayNameOffset[8]), nullptr, nullptr, 0);
    int32_t displayNameRead9 = SFileReadFile(file, &displayNameOffset[9], sizeof(displayNameOffset[9]), nullptr, nullptr, 0);
    int32_t displayNameRead10 = SFileReadFile(file, &displayNameOffset[10], sizeof(displayNameOffset[10]), nullptr, nullptr, 0);
    int32_t displayNameRead11 = SFileReadFile(file, &displayNameOffset[11], sizeof(displayNameOffset[11]), nullptr, nullptr, 0);
    int32_t displayNameRead12 = SFileReadFile(file, &displayNameOffset[12], sizeof(displayNameOffset[12]), nullptr, nullptr, 0);
    int32_t displayNameRead13 = SFileReadFile(file, &displayNameOffset[13], sizeof(displayNameOffset[13]), nullptr, nullptr, 0);
    int32_t displayNameRead14 = SFileReadFile(file, &displayNameOffset[14], sizeof(displayNameOffset[14]), nullptr, nullptr, 0);
    int32_t displayNameRead15 = SFileReadFile(file, &displayNameOffset[15], sizeof(displayNameOffset[15]), nullptr, nullptr, 0);
    int32_t displayNameFlagsRead = SFileReadFile(file, &displayNameFlags, sizeof(displayNameFlags), nullptr, nullptr, 0);
    int32_t verboseNameRead0 = SFileReadFile(file, &verboseNameOffset[0], sizeof(verboseNameOffset[0]), nullptr, nullptr, 0);
    int32_t verboseNameRead1 = SFileReadFile(file, &verboseNameOffset[1], sizeof(verboseNameOffset[1]), nullptr, nullptr, 0);
    int32_t verboseNameRead2 = SFileReadFile(file, &verboseNameOffset[2], sizeof(verboseNameOffset[2]), nullptr, nullptr, 0);
    int32_t verboseNameRead3 = SFileReadFile(file, &verboseNameOffset[3], sizeof(verboseNameOffset[3]), nullptr, nullptr, 0);
    int32_t verboseNameRead4 = SFileReadFile(file, &verboseNameOffset[4], sizeof(verboseNameOffset[4]), nullptr, nullptr, 0);
    int32_t verboseNameRead5 = SFileReadFile(file, &verboseNameOffset[5], sizeof(verboseNameOffset[5]), nullptr, nullptr, 0);
    int32_t verboseNameRead6 = SFileReadFile(file, &verboseNameOffset[6], sizeof(verboseNameOffset[6]), nullptr, nullptr, 0);
    int32_t verboseNameRead7 = SFileReadFile(file, &verboseNameOffset[7], sizeof(verboseNameOffset[7]), nullptr, nullptr, 0);
    int32_t verboseNameRead8 = SFileReadFile(file, &verboseNameOffset[8], sizeof(verboseNameOffset[8]), nullptr, nullptr, 0);
    int32_t verboseNameRead9 = SFileReadFile(file, &verboseNameOffset[9], sizeof(verboseNameOffset[9]), nullptr, nullptr, 0);
    int32_t verboseNameRead10 = SFileReadFile(file, &verboseNameOffset[10], sizeof(verboseNameOffset[10]), nullptr, nullptr, 0);
    int32_t verboseNameRead11 = SFileReadFile(file, &verboseNameOffset[11], sizeof(verboseNameOffset[11]), nullptr, nullptr, 0);
    int32_t verboseNameRead12 = SFileReadFile(file, &verboseNameOffset[12], sizeof(verboseNameOffset[12]), nullptr, nullptr, 0);
    int32_t verboseNameRead13 = SFileReadFile(file, &verboseNameOffset[13], sizeof(verboseNameOffset[13]), nullptr, nullptr, 0);
    int32_t verboseNameRead14 = SFileReadFile(file, &verboseNameOffset[14], sizeof(verboseNameOffset[14]), nullptr, nullptr, 0);
    int32_t verboseNameRead15 = SFileReadFile(file, &verboseNameOffset[15], sizeof(verboseNameOffset[15]), nullptr, nullptr, 0);
    int32_t verboseNameFlagsRead = SFileReadFile(file, &verboseNameFlags, sizeof(verboseNameFlags), nullptr, nullptr, 0);

    if (!classIDRead
        | !subClassIDRead
        | !prerequisiteProficiencyRead
        | !postrequisiteProficiencyRead
        | !flagsRead
        | !displayFlagsRead
        | !weaponParrySeqRead
        | !weaponReadySeqRead
        | !weaponAttackSeqRead
        | !weaponSwingSizeRead
        | !displayNameRead0
        | !displayNameRead1
        | !displayNameRead2
        | !displayNameRead3
        | !displayNameRead4
        | !displayNameRead5
        | !displayNameRead6
        | !displayNameRead7
        | !displayNameRead8
        | !displayNameRead9
        | !displayNameRead10
        | !displayNameRead11
        | !displayNameRead12
        | !displayNameRead13
        | !displayNameRead14
        | !displayNameRead15
        | !displayNameFlagsRead
        | !verboseNameRead0
        | !verboseNameRead1
        | !verboseNameRead2
        | !verboseNameRead3
        | !verboseNameRead4
        | !verboseNameRead5
        | !verboseNameRead6
        | !verboseNameRead7
        | !verboseNameRead8
        | !verboseNameRead9
        | !verboseNameRead10
        | !verboseNameRead11
        | !verboseNameRead12
        | !verboseNameRead13
        | !verboseNameRead14
        | !verboseNameRead15
        | !verboseNameFlagsRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_displayName = &stringBuffer[displayNameOffset[s_locale]];
        this->m_verboseName = &stringBuffer[verboseNameOffset[s_locale]];
    } else {
        this->m_displayName = "";
        this->m_verboseName = "";
    }

    return true;
}

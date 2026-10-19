#include "WoW/Source/DB/Rec/ChrClassesRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

ChrClassesRec::ChrClassesRec() {
}

ChrClassesRec::~ChrClassesRec() {
}

// VERIFY: a849b38c auto
// OFFSET: 0x008ABAA0
const char* ChrClassesRec::GetFilename() {
    return "DBFilesClient\\ChrClasses.dbc";
}

// VERIFY: 73ba26d1 auto
// OFFSET: 0x008ABAB0
bool ChrClassesRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t petNameTokenOffset;
    uint32_t nameOffset[16];
    uint32_t nameFlags;
    uint32_t nameFemaleOffset[16];
    uint32_t nameFemaleFlags;
    uint32_t nameMaleOffset[16];
    uint32_t nameMaleFlags;
    uint32_t filenameOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t damageBonusStatRead = SFileReadFile(file, &this->m_damageBonusStat, sizeof(this->m_damageBonusStat), nullptr, nullptr, 0);
    int32_t displayPowerRead = SFileReadFile(file, &this->m_displayPower, sizeof(this->m_displayPower), nullptr, nullptr, 0);
    int32_t petNameTokenRead = SFileReadFile(file, &petNameTokenOffset, sizeof(petNameTokenOffset), nullptr, nullptr, 0);
    int32_t nameRead0 = SFileReadFile(file, &nameOffset[0], sizeof(nameOffset[0]), nullptr, nullptr, 0);
    int32_t nameRead1 = SFileReadFile(file, &nameOffset[1], sizeof(nameOffset[1]), nullptr, nullptr, 0);
    int32_t nameRead2 = SFileReadFile(file, &nameOffset[2], sizeof(nameOffset[2]), nullptr, nullptr, 0);
    int32_t nameRead3 = SFileReadFile(file, &nameOffset[3], sizeof(nameOffset[3]), nullptr, nullptr, 0);
    int32_t nameRead4 = SFileReadFile(file, &nameOffset[4], sizeof(nameOffset[4]), nullptr, nullptr, 0);
    int32_t nameRead5 = SFileReadFile(file, &nameOffset[5], sizeof(nameOffset[5]), nullptr, nullptr, 0);
    int32_t nameRead6 = SFileReadFile(file, &nameOffset[6], sizeof(nameOffset[6]), nullptr, nullptr, 0);
    int32_t nameRead7 = SFileReadFile(file, &nameOffset[7], sizeof(nameOffset[7]), nullptr, nullptr, 0);
    int32_t nameRead8 = SFileReadFile(file, &nameOffset[8], sizeof(nameOffset[8]), nullptr, nullptr, 0);
    int32_t nameRead9 = SFileReadFile(file, &nameOffset[9], sizeof(nameOffset[9]), nullptr, nullptr, 0);
    int32_t nameRead10 = SFileReadFile(file, &nameOffset[10], sizeof(nameOffset[10]), nullptr, nullptr, 0);
    int32_t nameRead11 = SFileReadFile(file, &nameOffset[11], sizeof(nameOffset[11]), nullptr, nullptr, 0);
    int32_t nameRead12 = SFileReadFile(file, &nameOffset[12], sizeof(nameOffset[12]), nullptr, nullptr, 0);
    int32_t nameRead13 = SFileReadFile(file, &nameOffset[13], sizeof(nameOffset[13]), nullptr, nullptr, 0);
    int32_t nameRead14 = SFileReadFile(file, &nameOffset[14], sizeof(nameOffset[14]), nullptr, nullptr, 0);
    int32_t nameRead15 = SFileReadFile(file, &nameOffset[15], sizeof(nameOffset[15]), nullptr, nullptr, 0);
    int32_t nameFlagsRead = SFileReadFile(file, &nameFlags, sizeof(nameFlags), nullptr, nullptr, 0);
    int32_t nameFemaleRead0 = SFileReadFile(file, &nameFemaleOffset[0], sizeof(nameFemaleOffset[0]), nullptr, nullptr, 0);
    int32_t nameFemaleRead1 = SFileReadFile(file, &nameFemaleOffset[1], sizeof(nameFemaleOffset[1]), nullptr, nullptr, 0);
    int32_t nameFemaleRead2 = SFileReadFile(file, &nameFemaleOffset[2], sizeof(nameFemaleOffset[2]), nullptr, nullptr, 0);
    int32_t nameFemaleRead3 = SFileReadFile(file, &nameFemaleOffset[3], sizeof(nameFemaleOffset[3]), nullptr, nullptr, 0);
    int32_t nameFemaleRead4 = SFileReadFile(file, &nameFemaleOffset[4], sizeof(nameFemaleOffset[4]), nullptr, nullptr, 0);
    int32_t nameFemaleRead5 = SFileReadFile(file, &nameFemaleOffset[5], sizeof(nameFemaleOffset[5]), nullptr, nullptr, 0);
    int32_t nameFemaleRead6 = SFileReadFile(file, &nameFemaleOffset[6], sizeof(nameFemaleOffset[6]), nullptr, nullptr, 0);
    int32_t nameFemaleRead7 = SFileReadFile(file, &nameFemaleOffset[7], sizeof(nameFemaleOffset[7]), nullptr, nullptr, 0);
    int32_t nameFemaleRead8 = SFileReadFile(file, &nameFemaleOffset[8], sizeof(nameFemaleOffset[8]), nullptr, nullptr, 0);
    int32_t nameFemaleRead9 = SFileReadFile(file, &nameFemaleOffset[9], sizeof(nameFemaleOffset[9]), nullptr, nullptr, 0);
    int32_t nameFemaleRead10 = SFileReadFile(file, &nameFemaleOffset[10], sizeof(nameFemaleOffset[10]), nullptr, nullptr, 0);
    int32_t nameFemaleRead11 = SFileReadFile(file, &nameFemaleOffset[11], sizeof(nameFemaleOffset[11]), nullptr, nullptr, 0);
    int32_t nameFemaleRead12 = SFileReadFile(file, &nameFemaleOffset[12], sizeof(nameFemaleOffset[12]), nullptr, nullptr, 0);
    int32_t nameFemaleRead13 = SFileReadFile(file, &nameFemaleOffset[13], sizeof(nameFemaleOffset[13]), nullptr, nullptr, 0);
    int32_t nameFemaleRead14 = SFileReadFile(file, &nameFemaleOffset[14], sizeof(nameFemaleOffset[14]), nullptr, nullptr, 0);
    int32_t nameFemaleRead15 = SFileReadFile(file, &nameFemaleOffset[15], sizeof(nameFemaleOffset[15]), nullptr, nullptr, 0);
    int32_t nameFemaleFlagsRead = SFileReadFile(file, &nameFemaleFlags, sizeof(nameFemaleFlags), nullptr, nullptr, 0);
    int32_t nameMaleRead0 = SFileReadFile(file, &nameMaleOffset[0], sizeof(nameMaleOffset[0]), nullptr, nullptr, 0);
    int32_t nameMaleRead1 = SFileReadFile(file, &nameMaleOffset[1], sizeof(nameMaleOffset[1]), nullptr, nullptr, 0);
    int32_t nameMaleRead2 = SFileReadFile(file, &nameMaleOffset[2], sizeof(nameMaleOffset[2]), nullptr, nullptr, 0);
    int32_t nameMaleRead3 = SFileReadFile(file, &nameMaleOffset[3], sizeof(nameMaleOffset[3]), nullptr, nullptr, 0);
    int32_t nameMaleRead4 = SFileReadFile(file, &nameMaleOffset[4], sizeof(nameMaleOffset[4]), nullptr, nullptr, 0);
    int32_t nameMaleRead5 = SFileReadFile(file, &nameMaleOffset[5], sizeof(nameMaleOffset[5]), nullptr, nullptr, 0);
    int32_t nameMaleRead6 = SFileReadFile(file, &nameMaleOffset[6], sizeof(nameMaleOffset[6]), nullptr, nullptr, 0);
    int32_t nameMaleRead7 = SFileReadFile(file, &nameMaleOffset[7], sizeof(nameMaleOffset[7]), nullptr, nullptr, 0);
    int32_t nameMaleRead8 = SFileReadFile(file, &nameMaleOffset[8], sizeof(nameMaleOffset[8]), nullptr, nullptr, 0);
    int32_t nameMaleRead9 = SFileReadFile(file, &nameMaleOffset[9], sizeof(nameMaleOffset[9]), nullptr, nullptr, 0);
    int32_t nameMaleRead10 = SFileReadFile(file, &nameMaleOffset[10], sizeof(nameMaleOffset[10]), nullptr, nullptr, 0);
    int32_t nameMaleRead11 = SFileReadFile(file, &nameMaleOffset[11], sizeof(nameMaleOffset[11]), nullptr, nullptr, 0);
    int32_t nameMaleRead12 = SFileReadFile(file, &nameMaleOffset[12], sizeof(nameMaleOffset[12]), nullptr, nullptr, 0);
    int32_t nameMaleRead13 = SFileReadFile(file, &nameMaleOffset[13], sizeof(nameMaleOffset[13]), nullptr, nullptr, 0);
    int32_t nameMaleRead14 = SFileReadFile(file, &nameMaleOffset[14], sizeof(nameMaleOffset[14]), nullptr, nullptr, 0);
    int32_t nameMaleRead15 = SFileReadFile(file, &nameMaleOffset[15], sizeof(nameMaleOffset[15]), nullptr, nullptr, 0);
    int32_t nameMaleFlagsRead = SFileReadFile(file, &nameMaleFlags, sizeof(nameMaleFlags), nullptr, nullptr, 0);
    int32_t filenameRead = SFileReadFile(file, &filenameOffset, sizeof(filenameOffset), nullptr, nullptr, 0);
    int32_t spellClassSetRead = SFileReadFile(file, &this->m_spellClassSet, sizeof(this->m_spellClassSet), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t cinematicSequenceIDRead = SFileReadFile(file, &this->m_cinematicSequenceID, sizeof(this->m_cinematicSequenceID), nullptr, nullptr, 0);
    int32_t requiredExpansionRead = SFileReadFile(file, &this->m_requiredExpansion, sizeof(this->m_requiredExpansion), nullptr, nullptr, 0);

    if (!idRead
        | !damageBonusStatRead
        | !displayPowerRead
        | !petNameTokenRead
        | !nameRead0
        | !nameRead1
        | !nameRead2
        | !nameRead3
        | !nameRead4
        | !nameRead5
        | !nameRead6
        | !nameRead7
        | !nameRead8
        | !nameRead9
        | !nameRead10
        | !nameRead11
        | !nameRead12
        | !nameRead13
        | !nameRead14
        | !nameRead15
        | !nameFlagsRead
        | !nameFemaleRead0
        | !nameFemaleRead1
        | !nameFemaleRead2
        | !nameFemaleRead3
        | !nameFemaleRead4
        | !nameFemaleRead5
        | !nameFemaleRead6
        | !nameFemaleRead7
        | !nameFemaleRead8
        | !nameFemaleRead9
        | !nameFemaleRead10
        | !nameFemaleRead11
        | !nameFemaleRead12
        | !nameFemaleRead13
        | !nameFemaleRead14
        | !nameFemaleRead15
        | !nameFemaleFlagsRead
        | !nameMaleRead0
        | !nameMaleRead1
        | !nameMaleRead2
        | !nameMaleRead3
        | !nameMaleRead4
        | !nameMaleRead5
        | !nameMaleRead6
        | !nameMaleRead7
        | !nameMaleRead8
        | !nameMaleRead9
        | !nameMaleRead10
        | !nameMaleRead11
        | !nameMaleRead12
        | !nameMaleRead13
        | !nameMaleRead14
        | !nameMaleRead15
        | !nameMaleFlagsRead
        | !filenameRead
        | !spellClassSetRead
        | !flagsRead
        | !cinematicSequenceIDRead
        | !requiredExpansionRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_petNameToken = &stringBuffer[petNameTokenOffset];
        this->m_name = &stringBuffer[nameOffset[s_locale]];
        this->m_nameFemale = &stringBuffer[nameFemaleOffset[s_locale]];
        this->m_nameMale = &stringBuffer[nameMaleOffset[s_locale]];
        this->m_filename = &stringBuffer[filenameOffset];
    } else {
        this->m_petNameToken = "";
        this->m_name = "";
        this->m_nameFemale = "";
        this->m_nameMale = "";
        this->m_filename = "";
    }

    return true;
}

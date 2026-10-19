#include "WoW/Source/DB/Rec/WorldStateUIRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

WorldStateUIRec::WorldStateUIRec() {
}

WorldStateUIRec::~WorldStateUIRec() {
}

// VERIFY: 9e7cbbdd auto
// OFFSET: 0x008BC200
const char* WorldStateUIRec::GetFilename() {
    return "DBFilesClient\\WorldStateUI.dbc";
}

// VERIFY: 197732fc auto
// OFFSET: 0x008BC210
bool WorldStateUIRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t iconOffset;
    uint32_t stringOffset[16];
    uint32_t stringFlags;
    uint32_t tooltipOffset[16];
    uint32_t tooltipFlags;
    uint32_t dynamicIconOffset;
    uint32_t dynamicTooltipOffset[16];
    uint32_t dynamicTooltipFlags;
    uint32_t extendedUiOffset;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t mapIDRead = SFileReadFile(file, &this->m_mapID, sizeof(this->m_mapID), nullptr, nullptr, 0);
    int32_t areaIDRead = SFileReadFile(file, &this->m_areaID, sizeof(this->m_areaID), nullptr, nullptr, 0);
    int32_t phaseShiftRead = SFileReadFile(file, &this->m_phaseShift, sizeof(this->m_phaseShift), nullptr, nullptr, 0);
    int32_t iconRead = SFileReadFile(file, &iconOffset, sizeof(iconOffset), nullptr, nullptr, 0);
    int32_t stringRead0 = SFileReadFile(file, &stringOffset[0], sizeof(stringOffset[0]), nullptr, nullptr, 0);
    int32_t stringRead1 = SFileReadFile(file, &stringOffset[1], sizeof(stringOffset[1]), nullptr, nullptr, 0);
    int32_t stringRead2 = SFileReadFile(file, &stringOffset[2], sizeof(stringOffset[2]), nullptr, nullptr, 0);
    int32_t stringRead3 = SFileReadFile(file, &stringOffset[3], sizeof(stringOffset[3]), nullptr, nullptr, 0);
    int32_t stringRead4 = SFileReadFile(file, &stringOffset[4], sizeof(stringOffset[4]), nullptr, nullptr, 0);
    int32_t stringRead5 = SFileReadFile(file, &stringOffset[5], sizeof(stringOffset[5]), nullptr, nullptr, 0);
    int32_t stringRead6 = SFileReadFile(file, &stringOffset[6], sizeof(stringOffset[6]), nullptr, nullptr, 0);
    int32_t stringRead7 = SFileReadFile(file, &stringOffset[7], sizeof(stringOffset[7]), nullptr, nullptr, 0);
    int32_t stringRead8 = SFileReadFile(file, &stringOffset[8], sizeof(stringOffset[8]), nullptr, nullptr, 0);
    int32_t stringRead9 = SFileReadFile(file, &stringOffset[9], sizeof(stringOffset[9]), nullptr, nullptr, 0);
    int32_t stringRead10 = SFileReadFile(file, &stringOffset[10], sizeof(stringOffset[10]), nullptr, nullptr, 0);
    int32_t stringRead11 = SFileReadFile(file, &stringOffset[11], sizeof(stringOffset[11]), nullptr, nullptr, 0);
    int32_t stringRead12 = SFileReadFile(file, &stringOffset[12], sizeof(stringOffset[12]), nullptr, nullptr, 0);
    int32_t stringRead13 = SFileReadFile(file, &stringOffset[13], sizeof(stringOffset[13]), nullptr, nullptr, 0);
    int32_t stringRead14 = SFileReadFile(file, &stringOffset[14], sizeof(stringOffset[14]), nullptr, nullptr, 0);
    int32_t stringRead15 = SFileReadFile(file, &stringOffset[15], sizeof(stringOffset[15]), nullptr, nullptr, 0);
    int32_t stringFlagsRead = SFileReadFile(file, &stringFlags, sizeof(stringFlags), nullptr, nullptr, 0);
    int32_t tooltipRead0 = SFileReadFile(file, &tooltipOffset[0], sizeof(tooltipOffset[0]), nullptr, nullptr, 0);
    int32_t tooltipRead1 = SFileReadFile(file, &tooltipOffset[1], sizeof(tooltipOffset[1]), nullptr, nullptr, 0);
    int32_t tooltipRead2 = SFileReadFile(file, &tooltipOffset[2], sizeof(tooltipOffset[2]), nullptr, nullptr, 0);
    int32_t tooltipRead3 = SFileReadFile(file, &tooltipOffset[3], sizeof(tooltipOffset[3]), nullptr, nullptr, 0);
    int32_t tooltipRead4 = SFileReadFile(file, &tooltipOffset[4], sizeof(tooltipOffset[4]), nullptr, nullptr, 0);
    int32_t tooltipRead5 = SFileReadFile(file, &tooltipOffset[5], sizeof(tooltipOffset[5]), nullptr, nullptr, 0);
    int32_t tooltipRead6 = SFileReadFile(file, &tooltipOffset[6], sizeof(tooltipOffset[6]), nullptr, nullptr, 0);
    int32_t tooltipRead7 = SFileReadFile(file, &tooltipOffset[7], sizeof(tooltipOffset[7]), nullptr, nullptr, 0);
    int32_t tooltipRead8 = SFileReadFile(file, &tooltipOffset[8], sizeof(tooltipOffset[8]), nullptr, nullptr, 0);
    int32_t tooltipRead9 = SFileReadFile(file, &tooltipOffset[9], sizeof(tooltipOffset[9]), nullptr, nullptr, 0);
    int32_t tooltipRead10 = SFileReadFile(file, &tooltipOffset[10], sizeof(tooltipOffset[10]), nullptr, nullptr, 0);
    int32_t tooltipRead11 = SFileReadFile(file, &tooltipOffset[11], sizeof(tooltipOffset[11]), nullptr, nullptr, 0);
    int32_t tooltipRead12 = SFileReadFile(file, &tooltipOffset[12], sizeof(tooltipOffset[12]), nullptr, nullptr, 0);
    int32_t tooltipRead13 = SFileReadFile(file, &tooltipOffset[13], sizeof(tooltipOffset[13]), nullptr, nullptr, 0);
    int32_t tooltipRead14 = SFileReadFile(file, &tooltipOffset[14], sizeof(tooltipOffset[14]), nullptr, nullptr, 0);
    int32_t tooltipRead15 = SFileReadFile(file, &tooltipOffset[15], sizeof(tooltipOffset[15]), nullptr, nullptr, 0);
    int32_t tooltipFlagsRead = SFileReadFile(file, &tooltipFlags, sizeof(tooltipFlags), nullptr, nullptr, 0);
    int32_t stateVariableRead = SFileReadFile(file, &this->m_stateVariable, sizeof(this->m_stateVariable), nullptr, nullptr, 0);
    int32_t typeRead = SFileReadFile(file, &this->m_type, sizeof(this->m_type), nullptr, nullptr, 0);
    int32_t dynamicIconRead = SFileReadFile(file, &dynamicIconOffset, sizeof(dynamicIconOffset), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead0 = SFileReadFile(file, &dynamicTooltipOffset[0], sizeof(dynamicTooltipOffset[0]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead1 = SFileReadFile(file, &dynamicTooltipOffset[1], sizeof(dynamicTooltipOffset[1]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead2 = SFileReadFile(file, &dynamicTooltipOffset[2], sizeof(dynamicTooltipOffset[2]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead3 = SFileReadFile(file, &dynamicTooltipOffset[3], sizeof(dynamicTooltipOffset[3]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead4 = SFileReadFile(file, &dynamicTooltipOffset[4], sizeof(dynamicTooltipOffset[4]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead5 = SFileReadFile(file, &dynamicTooltipOffset[5], sizeof(dynamicTooltipOffset[5]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead6 = SFileReadFile(file, &dynamicTooltipOffset[6], sizeof(dynamicTooltipOffset[6]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead7 = SFileReadFile(file, &dynamicTooltipOffset[7], sizeof(dynamicTooltipOffset[7]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead8 = SFileReadFile(file, &dynamicTooltipOffset[8], sizeof(dynamicTooltipOffset[8]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead9 = SFileReadFile(file, &dynamicTooltipOffset[9], sizeof(dynamicTooltipOffset[9]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead10 = SFileReadFile(file, &dynamicTooltipOffset[10], sizeof(dynamicTooltipOffset[10]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead11 = SFileReadFile(file, &dynamicTooltipOffset[11], sizeof(dynamicTooltipOffset[11]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead12 = SFileReadFile(file, &dynamicTooltipOffset[12], sizeof(dynamicTooltipOffset[12]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead13 = SFileReadFile(file, &dynamicTooltipOffset[13], sizeof(dynamicTooltipOffset[13]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead14 = SFileReadFile(file, &dynamicTooltipOffset[14], sizeof(dynamicTooltipOffset[14]), nullptr, nullptr, 0);
    int32_t dynamicTooltipRead15 = SFileReadFile(file, &dynamicTooltipOffset[15], sizeof(dynamicTooltipOffset[15]), nullptr, nullptr, 0);
    int32_t dynamicTooltipFlagsRead = SFileReadFile(file, &dynamicTooltipFlags, sizeof(dynamicTooltipFlags), nullptr, nullptr, 0);
    int32_t extendedUiRead = SFileReadFile(file, &extendedUiOffset, sizeof(extendedUiOffset), nullptr, nullptr, 0);
    int32_t extendedUistateVariableRead = SFileReadFile(file, this->m_extendedUistateVariable, sizeof(this->m_extendedUistateVariable), nullptr, nullptr, 0);

    if (!idRead
        | !mapIDRead
        | !areaIDRead
        | !phaseShiftRead
        | !iconRead
        | !stringRead0
        | !stringRead1
        | !stringRead2
        | !stringRead3
        | !stringRead4
        | !stringRead5
        | !stringRead6
        | !stringRead7
        | !stringRead8
        | !stringRead9
        | !stringRead10
        | !stringRead11
        | !stringRead12
        | !stringRead13
        | !stringRead14
        | !stringRead15
        | !stringFlagsRead
        | !tooltipRead0
        | !tooltipRead1
        | !tooltipRead2
        | !tooltipRead3
        | !tooltipRead4
        | !tooltipRead5
        | !tooltipRead6
        | !tooltipRead7
        | !tooltipRead8
        | !tooltipRead9
        | !tooltipRead10
        | !tooltipRead11
        | !tooltipRead12
        | !tooltipRead13
        | !tooltipRead14
        | !tooltipRead15
        | !tooltipFlagsRead
        | !stateVariableRead
        | !typeRead
        | !dynamicIconRead
        | !dynamicTooltipRead0
        | !dynamicTooltipRead1
        | !dynamicTooltipRead2
        | !dynamicTooltipRead3
        | !dynamicTooltipRead4
        | !dynamicTooltipRead5
        | !dynamicTooltipRead6
        | !dynamicTooltipRead7
        | !dynamicTooltipRead8
        | !dynamicTooltipRead9
        | !dynamicTooltipRead10
        | !dynamicTooltipRead11
        | !dynamicTooltipRead12
        | !dynamicTooltipRead13
        | !dynamicTooltipRead14
        | !dynamicTooltipRead15
        | !dynamicTooltipFlagsRead
        | !extendedUiRead
        | !extendedUistateVariableRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_icon = &stringBuffer[iconOffset];
        this->m_string = &stringBuffer[stringOffset[s_locale]];
        this->m_tooltip = &stringBuffer[tooltipOffset[s_locale]];
        this->m_dynamicIcon = &stringBuffer[dynamicIconOffset];
        this->m_dynamicTooltip = &stringBuffer[dynamicTooltipOffset[s_locale]];
        this->m_extendedUi = &stringBuffer[extendedUiOffset];
    } else {
        this->m_icon = "";
        this->m_string = "";
        this->m_tooltip = "";
        this->m_dynamicIcon = "";
        this->m_dynamicTooltip = "";
        this->m_extendedUi = "";
    }

    return true;
}

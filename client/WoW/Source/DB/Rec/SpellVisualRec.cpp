#include "WoW/Source/DB/Rec/SpellVisualRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"

SpellVisualRec::SpellVisualRec() {
}

SpellVisualRec::~SpellVisualRec() {
}

// VERIFY: 1328342a auto
// OFFSET: 0x008B93E0
const char* SpellVisualRec::GetFilename() {
    return "DBFilesClient\\SpellVisual.dbc";
}

// VERIFY: acfbc15c auto
// OFFSET: 0x008B93F0
bool SpellVisualRec::Read(SFileImpl* file, const char* stringBuffer) {
    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t precastKitRead = SFileReadFile(file, &this->m_precastKit, sizeof(this->m_precastKit), nullptr, nullptr, 0);
    int32_t castKitRead = SFileReadFile(file, &this->m_castKit, sizeof(this->m_castKit), nullptr, nullptr, 0);
    int32_t impactKitRead = SFileReadFile(file, &this->m_impactKit, sizeof(this->m_impactKit), nullptr, nullptr, 0);
    int32_t stateKitRead = SFileReadFile(file, &this->m_stateKit, sizeof(this->m_stateKit), nullptr, nullptr, 0);
    int32_t stateDoneKitRead = SFileReadFile(file, &this->m_stateDoneKit, sizeof(this->m_stateDoneKit), nullptr, nullptr, 0);
    int32_t channelKitRead = SFileReadFile(file, &this->m_channelKit, sizeof(this->m_channelKit), nullptr, nullptr, 0);
    int32_t hasMissileRead = SFileReadFile(file, &this->m_hasMissile, sizeof(this->m_hasMissile), nullptr, nullptr, 0);
    int32_t missileModelRead = SFileReadFile(file, &this->m_missileModel, sizeof(this->m_missileModel), nullptr, nullptr, 0);
    int32_t missilePathTypeRead = SFileReadFile(file, &this->m_missilePathType, sizeof(this->m_missilePathType), nullptr, nullptr, 0);
    int32_t missileDestinationAttachmentRead = SFileReadFile(file, &this->m_missileDestinationAttachment, sizeof(this->m_missileDestinationAttachment), nullptr, nullptr, 0);
    int32_t missileSoundRead = SFileReadFile(file, &this->m_missileSound, sizeof(this->m_missileSound), nullptr, nullptr, 0);
    int32_t animEventSoundIDRead = SFileReadFile(file, &this->m_animEventSoundID, sizeof(this->m_animEventSoundID), nullptr, nullptr, 0);
    int32_t flagsRead = SFileReadFile(file, &this->m_flags, sizeof(this->m_flags), nullptr, nullptr, 0);
    int32_t casterImpactKitRead = SFileReadFile(file, &this->m_casterImpactKit, sizeof(this->m_casterImpactKit), nullptr, nullptr, 0);
    int32_t targetImpactKitRead = SFileReadFile(file, &this->m_targetImpactKit, sizeof(this->m_targetImpactKit), nullptr, nullptr, 0);
    int32_t missileAttachmentRead = SFileReadFile(file, &this->m_missileAttachment, sizeof(this->m_missileAttachment), nullptr, nullptr, 0);
    int32_t missileFollowGroundHeightRead = SFileReadFile(file, &this->m_missileFollowGroundHeight, sizeof(this->m_missileFollowGroundHeight), nullptr, nullptr, 0);
    int32_t missileFollowGroundDropSpeedRead = SFileReadFile(file, &this->m_missileFollowGroundDropSpeed, sizeof(this->m_missileFollowGroundDropSpeed), nullptr, nullptr, 0);
    int32_t missileFollowGroundApproachRead = SFileReadFile(file, &this->m_missileFollowGroundApproach, sizeof(this->m_missileFollowGroundApproach), nullptr, nullptr, 0);
    int32_t missileFollowGroundFlagsRead = SFileReadFile(file, &this->m_missileFollowGroundFlags, sizeof(this->m_missileFollowGroundFlags), nullptr, nullptr, 0);
    int32_t missileMotionRead = SFileReadFile(file, &this->m_missileMotion, sizeof(this->m_missileMotion), nullptr, nullptr, 0);
    int32_t missileTargetingKitRead = SFileReadFile(file, &this->m_missileTargetingKit, sizeof(this->m_missileTargetingKit), nullptr, nullptr, 0);
    int32_t instantAreaKitRead = SFileReadFile(file, &this->m_instantAreaKit, sizeof(this->m_instantAreaKit), nullptr, nullptr, 0);
    int32_t impactAreaKitRead = SFileReadFile(file, &this->m_impactAreaKit, sizeof(this->m_impactAreaKit), nullptr, nullptr, 0);
    int32_t persistentAreaKitRead = SFileReadFile(file, &this->m_persistentAreaKit, sizeof(this->m_persistentAreaKit), nullptr, nullptr, 0);
    int32_t missileCastRead0 = SFileReadFile(file, &this->m_missileCastOffset[0], sizeof(this->m_missileCastOffset[0]), nullptr, nullptr, 0);
    int32_t missileCastRead1 = SFileReadFile(file, &this->m_missileCastOffset[1], sizeof(this->m_missileCastOffset[1]), nullptr, nullptr, 0);
    int32_t missileCastRead2 = SFileReadFile(file, &this->m_missileCastOffset[2], sizeof(this->m_missileCastOffset[2]), nullptr, nullptr, 0);
    int32_t missileImpactRead0 = SFileReadFile(file, &this->m_missileImpactOffset[0], sizeof(this->m_missileImpactOffset[0]), nullptr, nullptr, 0);
    int32_t missileImpactRead1 = SFileReadFile(file, &this->m_missileImpactOffset[1], sizeof(this->m_missileImpactOffset[1]), nullptr, nullptr, 0);
    int32_t missileImpactRead2 = SFileReadFile(file, &this->m_missileImpactOffset[2], sizeof(this->m_missileImpactOffset[2]), nullptr, nullptr, 0);

    if (!idRead
        | !precastKitRead
        | !castKitRead
        | !impactKitRead
        | !stateKitRead
        | !stateDoneKitRead
        | !channelKitRead
        | !hasMissileRead
        | !missileModelRead
        | !missilePathTypeRead
        | !missileDestinationAttachmentRead
        | !missileSoundRead
        | !animEventSoundIDRead
        | !flagsRead
        | !casterImpactKitRead
        | !targetImpactKitRead
        | !missileAttachmentRead
        | !missileFollowGroundHeightRead
        | !missileFollowGroundDropSpeedRead
        | !missileFollowGroundApproachRead
        | !missileFollowGroundFlagsRead
        | !missileMotionRead
        | !missileTargetingKitRead
        | !instantAreaKitRead
        | !impactAreaKitRead
        | !persistentAreaKitRead
        | !missileCastRead0
        | !missileCastRead1
        | !missileCastRead2
        | !missileImpactRead0
        | !missileImpactRead1
        | !missileImpactRead2) {
        return false;
    }

    return true;
}

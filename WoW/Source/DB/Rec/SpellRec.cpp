#include "WoW/Source/DB/Rec/SpellRec.h"
#include "Engine/Source/BlizzardCore/SFile2-Core.h"
#include "WoW/Source/Client.h"

SpellRec::SpellRec() {
}

SpellRec::~SpellRec() {
}

// VERIFY: fb692a85 auto
// OFFSET: 0x008A65D0
const char* SpellRec::GetFilename() {
    return "DBFilesClient\\Spell.dbc";
}

// VERIFY: b010564b auto
// OFFSET: 0x008A65E0
bool SpellRec::Read(SFileImpl* file, const char* stringBuffer) {
    uint32_t nameOffset[16];
    uint32_t nameFlags;
    uint32_t nameSubtextOffset[16];
    uint32_t nameSubtextFlags;
    uint32_t descriptionOffset[16];
    uint32_t descriptionFlags;
    uint32_t auraDescriptionOffset[16];
    uint32_t auraDescriptionFlags;

    int32_t idRead = SFileReadFile(file, &this->m_id, sizeof(this->m_id), nullptr, nullptr, 0);
    int32_t categoryRead = SFileReadFile(file, &this->m_category, sizeof(this->m_category), nullptr, nullptr, 0);
    int32_t dispelTypeRead = SFileReadFile(file, &this->m_dispelType, sizeof(this->m_dispelType), nullptr, nullptr, 0);
    int32_t mechanicRead = SFileReadFile(file, &this->m_mechanic, sizeof(this->m_mechanic), nullptr, nullptr, 0);
    int32_t attributesRead = SFileReadFile(file, &this->m_attributes, sizeof(this->m_attributes), nullptr, nullptr, 0);
    int32_t attributesExRead = SFileReadFile(file, &this->m_attributesEx, sizeof(this->m_attributesEx), nullptr, nullptr, 0);
    int32_t attributesExBRead = SFileReadFile(file, &this->m_attributesExB, sizeof(this->m_attributesExB), nullptr, nullptr, 0);
    int32_t attributesExCRead = SFileReadFile(file, &this->m_attributesExC, sizeof(this->m_attributesExC), nullptr, nullptr, 0);
    int32_t attributesExDRead = SFileReadFile(file, &this->m_attributesExD, sizeof(this->m_attributesExD), nullptr, nullptr, 0);
    int32_t attributesExERead = SFileReadFile(file, &this->m_attributesExE, sizeof(this->m_attributesExE), nullptr, nullptr, 0);
    int32_t attributesExFRead = SFileReadFile(file, &this->m_attributesExF, sizeof(this->m_attributesExF), nullptr, nullptr, 0);
    int32_t attributesExGRead = SFileReadFile(file, &this->m_attributesExG, sizeof(this->m_attributesExG), nullptr, nullptr, 0);
    int32_t shapeshiftMaskRead = SFileReadFile(file, this->m_shapeshiftMask, sizeof(this->m_shapeshiftMask), nullptr, nullptr, 0);
    int32_t shapeshiftExcludeRead = SFileReadFile(file, this->m_shapeshiftExclude, sizeof(this->m_shapeshiftExclude), nullptr, nullptr, 0);
    int32_t targetsRead = SFileReadFile(file, &this->m_targets, sizeof(this->m_targets), nullptr, nullptr, 0);
    int32_t targetCreatureTypeRead = SFileReadFile(file, &this->m_targetCreatureType, sizeof(this->m_targetCreatureType), nullptr, nullptr, 0);
    int32_t requiresSpellFocusRead = SFileReadFile(file, &this->m_requiresSpellFocus, sizeof(this->m_requiresSpellFocus), nullptr, nullptr, 0);
    int32_t facingCasterFlagsRead = SFileReadFile(file, &this->m_facingCasterFlags, sizeof(this->m_facingCasterFlags), nullptr, nullptr, 0);
    int32_t casterAuraStateRead = SFileReadFile(file, &this->m_casterAuraState, sizeof(this->m_casterAuraState), nullptr, nullptr, 0);
    int32_t targetAuraStateRead = SFileReadFile(file, &this->m_targetAuraState, sizeof(this->m_targetAuraState), nullptr, nullptr, 0);
    int32_t excludeCasterAuraStateRead = SFileReadFile(file, &this->m_excludeCasterAuraState, sizeof(this->m_excludeCasterAuraState), nullptr, nullptr, 0);
    int32_t excludeTargetAuraStateRead = SFileReadFile(file, &this->m_excludeTargetAuraState, sizeof(this->m_excludeTargetAuraState), nullptr, nullptr, 0);
    int32_t casterAuraSpellRead = SFileReadFile(file, &this->m_casterAuraSpell, sizeof(this->m_casterAuraSpell), nullptr, nullptr, 0);
    int32_t targetAuraSpellRead = SFileReadFile(file, &this->m_targetAuraSpell, sizeof(this->m_targetAuraSpell), nullptr, nullptr, 0);
    int32_t excludeCasterAuraSpellRead = SFileReadFile(file, &this->m_excludeCasterAuraSpell, sizeof(this->m_excludeCasterAuraSpell), nullptr, nullptr, 0);
    int32_t excludeTargetAuraSpellRead = SFileReadFile(file, &this->m_excludeTargetAuraSpell, sizeof(this->m_excludeTargetAuraSpell), nullptr, nullptr, 0);
    int32_t castingTimeIndexRead = SFileReadFile(file, &this->m_castingTimeIndex, sizeof(this->m_castingTimeIndex), nullptr, nullptr, 0);
    int32_t recoveryTimeRead = SFileReadFile(file, &this->m_recoveryTime, sizeof(this->m_recoveryTime), nullptr, nullptr, 0);
    int32_t categoryRecoveryTimeRead = SFileReadFile(file, &this->m_categoryRecoveryTime, sizeof(this->m_categoryRecoveryTime), nullptr, nullptr, 0);
    int32_t interruptFlagsRead = SFileReadFile(file, &this->m_interruptFlags, sizeof(this->m_interruptFlags), nullptr, nullptr, 0);
    int32_t auraInterruptFlagsRead = SFileReadFile(file, &this->m_auraInterruptFlags, sizeof(this->m_auraInterruptFlags), nullptr, nullptr, 0);
    int32_t channelInterruptFlagsRead = SFileReadFile(file, &this->m_channelInterruptFlags, sizeof(this->m_channelInterruptFlags), nullptr, nullptr, 0);
    int32_t procTypeMaskRead = SFileReadFile(file, &this->m_procTypeMask, sizeof(this->m_procTypeMask), nullptr, nullptr, 0);
    int32_t procChanceRead = SFileReadFile(file, &this->m_procChance, sizeof(this->m_procChance), nullptr, nullptr, 0);
    int32_t procChargesRead = SFileReadFile(file, &this->m_procCharges, sizeof(this->m_procCharges), nullptr, nullptr, 0);
    int32_t maxLevelRead = SFileReadFile(file, &this->m_maxLevel, sizeof(this->m_maxLevel), nullptr, nullptr, 0);
    int32_t baseLevelRead = SFileReadFile(file, &this->m_baseLevel, sizeof(this->m_baseLevel), nullptr, nullptr, 0);
    int32_t spellLevelRead = SFileReadFile(file, &this->m_spellLevel, sizeof(this->m_spellLevel), nullptr, nullptr, 0);
    int32_t durationIndexRead = SFileReadFile(file, &this->m_durationIndex, sizeof(this->m_durationIndex), nullptr, nullptr, 0);
    int32_t powerTypeRead = SFileReadFile(file, &this->m_powerType, sizeof(this->m_powerType), nullptr, nullptr, 0);
    int32_t manaCostRead = SFileReadFile(file, &this->m_manaCost, sizeof(this->m_manaCost), nullptr, nullptr, 0);
    int32_t manaCostPerLevelRead = SFileReadFile(file, &this->m_manaCostPerLevel, sizeof(this->m_manaCostPerLevel), nullptr, nullptr, 0);
    int32_t manaPerSecondRead = SFileReadFile(file, &this->m_manaPerSecond, sizeof(this->m_manaPerSecond), nullptr, nullptr, 0);
    int32_t manaPerSecondPerLevelRead = SFileReadFile(file, &this->m_manaPerSecondPerLevel, sizeof(this->m_manaPerSecondPerLevel), nullptr, nullptr, 0);
    int32_t rangeIndexRead = SFileReadFile(file, &this->m_rangeIndex, sizeof(this->m_rangeIndex), nullptr, nullptr, 0);
    int32_t speedRead = SFileReadFile(file, &this->m_speed, sizeof(this->m_speed), nullptr, nullptr, 0);
    int32_t modalNextSpellRead = SFileReadFile(file, &this->m_modalNextSpell, sizeof(this->m_modalNextSpell), nullptr, nullptr, 0);
    int32_t cumulativeAuraRead = SFileReadFile(file, &this->m_cumulativeAura, sizeof(this->m_cumulativeAura), nullptr, nullptr, 0);
    int32_t totemRead = SFileReadFile(file, this->m_totem, sizeof(this->m_totem), nullptr, nullptr, 0);
    int32_t reagentRead = SFileReadFile(file, this->m_reagent, sizeof(this->m_reagent), nullptr, nullptr, 0);
    int32_t reagentCountRead = SFileReadFile(file, this->m_reagentCount, sizeof(this->m_reagentCount), nullptr, nullptr, 0);
    int32_t equippedItemClassRead = SFileReadFile(file, &this->m_equippedItemClass, sizeof(this->m_equippedItemClass), nullptr, nullptr, 0);
    int32_t equippedItemSubclassRead = SFileReadFile(file, &this->m_equippedItemSubclass, sizeof(this->m_equippedItemSubclass), nullptr, nullptr, 0);
    int32_t equippedItemInvTypesRead = SFileReadFile(file, &this->m_equippedItemInvTypes, sizeof(this->m_equippedItemInvTypes), nullptr, nullptr, 0);
    int32_t effectRead = SFileReadFile(file, this->m_effect, sizeof(this->m_effect), nullptr, nullptr, 0);
    int32_t effectDieSidesRead = SFileReadFile(file, this->m_effectDieSides, sizeof(this->m_effectDieSides), nullptr, nullptr, 0);
    int32_t effectRealPointsPerLevelRead = SFileReadFile(file, this->m_effectRealPointsPerLevel, sizeof(this->m_effectRealPointsPerLevel), nullptr, nullptr, 0);
    int32_t effectBasePointsRead = SFileReadFile(file, this->m_effectBasePoints, sizeof(this->m_effectBasePoints), nullptr, nullptr, 0);
    int32_t effectMechanicRead = SFileReadFile(file, this->m_effectMechanic, sizeof(this->m_effectMechanic), nullptr, nullptr, 0);
    int32_t implicitTargetARead = SFileReadFile(file, this->m_implicitTargetA, sizeof(this->m_implicitTargetA), nullptr, nullptr, 0);
    int32_t implicitTargetBRead = SFileReadFile(file, this->m_implicitTargetB, sizeof(this->m_implicitTargetB), nullptr, nullptr, 0);
    int32_t effectRadiusIndexRead = SFileReadFile(file, this->m_effectRadiusIndex, sizeof(this->m_effectRadiusIndex), nullptr, nullptr, 0);
    int32_t effectAuraRead = SFileReadFile(file, this->m_effectAura, sizeof(this->m_effectAura), nullptr, nullptr, 0);
    int32_t effectAuraPeriodRead = SFileReadFile(file, this->m_effectAuraPeriod, sizeof(this->m_effectAuraPeriod), nullptr, nullptr, 0);
    int32_t effectAmplitudeRead = SFileReadFile(file, this->m_effectAmplitude, sizeof(this->m_effectAmplitude), nullptr, nullptr, 0);
    int32_t effectChainTargetsRead = SFileReadFile(file, this->m_effectChainTargets, sizeof(this->m_effectChainTargets), nullptr, nullptr, 0);
    int32_t effectItemTypeRead = SFileReadFile(file, this->m_effectItemType, sizeof(this->m_effectItemType), nullptr, nullptr, 0);
    int32_t effectMiscValueRead = SFileReadFile(file, this->m_effectMiscValue, sizeof(this->m_effectMiscValue), nullptr, nullptr, 0);
    int32_t effectMiscValueBRead = SFileReadFile(file, this->m_effectMiscValueB, sizeof(this->m_effectMiscValueB), nullptr, nullptr, 0);
    int32_t effectTriggerSpellRead = SFileReadFile(file, this->m_effectTriggerSpell, sizeof(this->m_effectTriggerSpell), nullptr, nullptr, 0);
    int32_t effectPointsPerComboRead = SFileReadFile(file, this->m_effectPointsPerCombo, sizeof(this->m_effectPointsPerCombo), nullptr, nullptr, 0);
    int32_t effectSpellClassMaskARead = SFileReadFile(file, this->m_effectSpellClassMaskA, sizeof(this->m_effectSpellClassMaskA), nullptr, nullptr, 0);
    int32_t effectSpellClassMaskBRead = SFileReadFile(file, this->m_effectSpellClassMaskB, sizeof(this->m_effectSpellClassMaskB), nullptr, nullptr, 0);
    int32_t effectSpellClassMaskCRead = SFileReadFile(file, this->m_effectSpellClassMaskC, sizeof(this->m_effectSpellClassMaskC), nullptr, nullptr, 0);
    int32_t spellVisualIDRead = SFileReadFile(file, this->m_spellVisualID, sizeof(this->m_spellVisualID), nullptr, nullptr, 0);
    int32_t spellIconIDRead = SFileReadFile(file, &this->m_spellIconID, sizeof(this->m_spellIconID), nullptr, nullptr, 0);
    int32_t activeIconIDRead = SFileReadFile(file, &this->m_activeIconID, sizeof(this->m_activeIconID), nullptr, nullptr, 0);
    int32_t spellPriorityRead = SFileReadFile(file, &this->m_spellPriority, sizeof(this->m_spellPriority), nullptr, nullptr, 0);
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
    int32_t nameSubtextRead0 = SFileReadFile(file, &nameSubtextOffset[0], sizeof(nameSubtextOffset[0]), nullptr, nullptr, 0);
    int32_t nameSubtextRead1 = SFileReadFile(file, &nameSubtextOffset[1], sizeof(nameSubtextOffset[1]), nullptr, nullptr, 0);
    int32_t nameSubtextRead2 = SFileReadFile(file, &nameSubtextOffset[2], sizeof(nameSubtextOffset[2]), nullptr, nullptr, 0);
    int32_t nameSubtextRead3 = SFileReadFile(file, &nameSubtextOffset[3], sizeof(nameSubtextOffset[3]), nullptr, nullptr, 0);
    int32_t nameSubtextRead4 = SFileReadFile(file, &nameSubtextOffset[4], sizeof(nameSubtextOffset[4]), nullptr, nullptr, 0);
    int32_t nameSubtextRead5 = SFileReadFile(file, &nameSubtextOffset[5], sizeof(nameSubtextOffset[5]), nullptr, nullptr, 0);
    int32_t nameSubtextRead6 = SFileReadFile(file, &nameSubtextOffset[6], sizeof(nameSubtextOffset[6]), nullptr, nullptr, 0);
    int32_t nameSubtextRead7 = SFileReadFile(file, &nameSubtextOffset[7], sizeof(nameSubtextOffset[7]), nullptr, nullptr, 0);
    int32_t nameSubtextRead8 = SFileReadFile(file, &nameSubtextOffset[8], sizeof(nameSubtextOffset[8]), nullptr, nullptr, 0);
    int32_t nameSubtextRead9 = SFileReadFile(file, &nameSubtextOffset[9], sizeof(nameSubtextOffset[9]), nullptr, nullptr, 0);
    int32_t nameSubtextRead10 = SFileReadFile(file, &nameSubtextOffset[10], sizeof(nameSubtextOffset[10]), nullptr, nullptr, 0);
    int32_t nameSubtextRead11 = SFileReadFile(file, &nameSubtextOffset[11], sizeof(nameSubtextOffset[11]), nullptr, nullptr, 0);
    int32_t nameSubtextRead12 = SFileReadFile(file, &nameSubtextOffset[12], sizeof(nameSubtextOffset[12]), nullptr, nullptr, 0);
    int32_t nameSubtextRead13 = SFileReadFile(file, &nameSubtextOffset[13], sizeof(nameSubtextOffset[13]), nullptr, nullptr, 0);
    int32_t nameSubtextRead14 = SFileReadFile(file, &nameSubtextOffset[14], sizeof(nameSubtextOffset[14]), nullptr, nullptr, 0);
    int32_t nameSubtextRead15 = SFileReadFile(file, &nameSubtextOffset[15], sizeof(nameSubtextOffset[15]), nullptr, nullptr, 0);
    int32_t nameSubtextFlagsRead = SFileReadFile(file, &nameSubtextFlags, sizeof(nameSubtextFlags), nullptr, nullptr, 0);
    int32_t descriptionRead0 = SFileReadFile(file, &descriptionOffset[0], sizeof(descriptionOffset[0]), nullptr, nullptr, 0);
    int32_t descriptionRead1 = SFileReadFile(file, &descriptionOffset[1], sizeof(descriptionOffset[1]), nullptr, nullptr, 0);
    int32_t descriptionRead2 = SFileReadFile(file, &descriptionOffset[2], sizeof(descriptionOffset[2]), nullptr, nullptr, 0);
    int32_t descriptionRead3 = SFileReadFile(file, &descriptionOffset[3], sizeof(descriptionOffset[3]), nullptr, nullptr, 0);
    int32_t descriptionRead4 = SFileReadFile(file, &descriptionOffset[4], sizeof(descriptionOffset[4]), nullptr, nullptr, 0);
    int32_t descriptionRead5 = SFileReadFile(file, &descriptionOffset[5], sizeof(descriptionOffset[5]), nullptr, nullptr, 0);
    int32_t descriptionRead6 = SFileReadFile(file, &descriptionOffset[6], sizeof(descriptionOffset[6]), nullptr, nullptr, 0);
    int32_t descriptionRead7 = SFileReadFile(file, &descriptionOffset[7], sizeof(descriptionOffset[7]), nullptr, nullptr, 0);
    int32_t descriptionRead8 = SFileReadFile(file, &descriptionOffset[8], sizeof(descriptionOffset[8]), nullptr, nullptr, 0);
    int32_t descriptionRead9 = SFileReadFile(file, &descriptionOffset[9], sizeof(descriptionOffset[9]), nullptr, nullptr, 0);
    int32_t descriptionRead10 = SFileReadFile(file, &descriptionOffset[10], sizeof(descriptionOffset[10]), nullptr, nullptr, 0);
    int32_t descriptionRead11 = SFileReadFile(file, &descriptionOffset[11], sizeof(descriptionOffset[11]), nullptr, nullptr, 0);
    int32_t descriptionRead12 = SFileReadFile(file, &descriptionOffset[12], sizeof(descriptionOffset[12]), nullptr, nullptr, 0);
    int32_t descriptionRead13 = SFileReadFile(file, &descriptionOffset[13], sizeof(descriptionOffset[13]), nullptr, nullptr, 0);
    int32_t descriptionRead14 = SFileReadFile(file, &descriptionOffset[14], sizeof(descriptionOffset[14]), nullptr, nullptr, 0);
    int32_t descriptionRead15 = SFileReadFile(file, &descriptionOffset[15], sizeof(descriptionOffset[15]), nullptr, nullptr, 0);
    int32_t descriptionFlagsRead = SFileReadFile(file, &descriptionFlags, sizeof(descriptionFlags), nullptr, nullptr, 0);
    int32_t auraDescriptionRead0 = SFileReadFile(file, &auraDescriptionOffset[0], sizeof(auraDescriptionOffset[0]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead1 = SFileReadFile(file, &auraDescriptionOffset[1], sizeof(auraDescriptionOffset[1]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead2 = SFileReadFile(file, &auraDescriptionOffset[2], sizeof(auraDescriptionOffset[2]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead3 = SFileReadFile(file, &auraDescriptionOffset[3], sizeof(auraDescriptionOffset[3]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead4 = SFileReadFile(file, &auraDescriptionOffset[4], sizeof(auraDescriptionOffset[4]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead5 = SFileReadFile(file, &auraDescriptionOffset[5], sizeof(auraDescriptionOffset[5]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead6 = SFileReadFile(file, &auraDescriptionOffset[6], sizeof(auraDescriptionOffset[6]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead7 = SFileReadFile(file, &auraDescriptionOffset[7], sizeof(auraDescriptionOffset[7]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead8 = SFileReadFile(file, &auraDescriptionOffset[8], sizeof(auraDescriptionOffset[8]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead9 = SFileReadFile(file, &auraDescriptionOffset[9], sizeof(auraDescriptionOffset[9]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead10 = SFileReadFile(file, &auraDescriptionOffset[10], sizeof(auraDescriptionOffset[10]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead11 = SFileReadFile(file, &auraDescriptionOffset[11], sizeof(auraDescriptionOffset[11]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead12 = SFileReadFile(file, &auraDescriptionOffset[12], sizeof(auraDescriptionOffset[12]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead13 = SFileReadFile(file, &auraDescriptionOffset[13], sizeof(auraDescriptionOffset[13]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead14 = SFileReadFile(file, &auraDescriptionOffset[14], sizeof(auraDescriptionOffset[14]), nullptr, nullptr, 0);
    int32_t auraDescriptionRead15 = SFileReadFile(file, &auraDescriptionOffset[15], sizeof(auraDescriptionOffset[15]), nullptr, nullptr, 0);
    int32_t auraDescriptionFlagsRead = SFileReadFile(file, &auraDescriptionFlags, sizeof(auraDescriptionFlags), nullptr, nullptr, 0);
    int32_t manaCostPctRead = SFileReadFile(file, &this->m_manaCostPct, sizeof(this->m_manaCostPct), nullptr, nullptr, 0);
    int32_t startRecoveryCategoryRead = SFileReadFile(file, &this->m_startRecoveryCategory, sizeof(this->m_startRecoveryCategory), nullptr, nullptr, 0);
    int32_t startRecoveryTimeRead = SFileReadFile(file, &this->m_startRecoveryTime, sizeof(this->m_startRecoveryTime), nullptr, nullptr, 0);
    int32_t maxTargetLevelRead = SFileReadFile(file, &this->m_maxTargetLevel, sizeof(this->m_maxTargetLevel), nullptr, nullptr, 0);
    int32_t spellClassSetRead = SFileReadFile(file, &this->m_spellClassSet, sizeof(this->m_spellClassSet), nullptr, nullptr, 0);
    int32_t spellClassMaskRead = SFileReadFile(file, this->m_spellClassMask, sizeof(this->m_spellClassMask), nullptr, nullptr, 0);
    int32_t maxTargetsRead = SFileReadFile(file, &this->m_maxTargets, sizeof(this->m_maxTargets), nullptr, nullptr, 0);
    int32_t defenseTypeRead = SFileReadFile(file, &this->m_defenseType, sizeof(this->m_defenseType), nullptr, nullptr, 0);
    int32_t preventionTypeRead = SFileReadFile(file, &this->m_preventionType, sizeof(this->m_preventionType), nullptr, nullptr, 0);
    int32_t stanceBarOrderRead = SFileReadFile(file, &this->m_stanceBarOrder, sizeof(this->m_stanceBarOrder), nullptr, nullptr, 0);
    int32_t effectChainAmplitudeRead = SFileReadFile(file, this->m_effectChainAmplitude, sizeof(this->m_effectChainAmplitude), nullptr, nullptr, 0);
    int32_t minFactionIDRead = SFileReadFile(file, &this->m_minFactionID, sizeof(this->m_minFactionID), nullptr, nullptr, 0);
    int32_t minReputationRead = SFileReadFile(file, &this->m_minReputation, sizeof(this->m_minReputation), nullptr, nullptr, 0);
    int32_t requiredAuraVisionRead = SFileReadFile(file, &this->m_requiredAuraVision, sizeof(this->m_requiredAuraVision), nullptr, nullptr, 0);
    int32_t requiredTotemCategoryIDRead = SFileReadFile(file, this->m_requiredTotemCategoryID, sizeof(this->m_requiredTotemCategoryID), nullptr, nullptr, 0);
    int32_t requiredAreasIDRead = SFileReadFile(file, &this->m_requiredAreasID, sizeof(this->m_requiredAreasID), nullptr, nullptr, 0);
    int32_t schoolMaskRead = SFileReadFile(file, &this->m_schoolMask, sizeof(this->m_schoolMask), nullptr, nullptr, 0);
    int32_t runeCostIDRead = SFileReadFile(file, &this->m_runeCostID, sizeof(this->m_runeCostID), nullptr, nullptr, 0);
    int32_t spellMissileIDRead = SFileReadFile(file, &this->m_spellMissileID, sizeof(this->m_spellMissileID), nullptr, nullptr, 0);
    int32_t powerDisplayIDRead = SFileReadFile(file, &this->m_powerDisplayID, sizeof(this->m_powerDisplayID), nullptr, nullptr, 0);
    int32_t effectBonusCoefficientRead = SFileReadFile(file, this->m_effectBonusCoefficient, sizeof(this->m_effectBonusCoefficient), nullptr, nullptr, 0);
    int32_t descriptionVariablesIDRead = SFileReadFile(file, &this->m_descriptionVariablesID, sizeof(this->m_descriptionVariablesID), nullptr, nullptr, 0);
    int32_t difficultyRead = SFileReadFile(file, &this->m_difficulty, sizeof(this->m_difficulty), nullptr, nullptr, 0);

    if (!idRead
        | !categoryRead
        | !dispelTypeRead
        | !mechanicRead
        | !attributesRead
        | !attributesExRead
        | !attributesExBRead
        | !attributesExCRead
        | !attributesExDRead
        | !attributesExERead
        | !attributesExFRead
        | !attributesExGRead
        | !shapeshiftMaskRead
        | !shapeshiftExcludeRead
        | !targetsRead
        | !targetCreatureTypeRead
        | !requiresSpellFocusRead
        | !facingCasterFlagsRead
        | !casterAuraStateRead
        | !targetAuraStateRead
        | !excludeCasterAuraStateRead
        | !excludeTargetAuraStateRead
        | !casterAuraSpellRead
        | !targetAuraSpellRead
        | !excludeCasterAuraSpellRead
        | !excludeTargetAuraSpellRead
        | !castingTimeIndexRead
        | !recoveryTimeRead
        | !categoryRecoveryTimeRead
        | !interruptFlagsRead
        | !auraInterruptFlagsRead
        | !channelInterruptFlagsRead
        | !procTypeMaskRead
        | !procChanceRead
        | !procChargesRead
        | !maxLevelRead
        | !baseLevelRead
        | !spellLevelRead
        | !durationIndexRead
        | !powerTypeRead
        | !manaCostRead
        | !manaCostPerLevelRead
        | !manaPerSecondRead
        | !manaPerSecondPerLevelRead
        | !rangeIndexRead
        | !speedRead
        | !modalNextSpellRead
        | !cumulativeAuraRead
        | !totemRead
        | !reagentRead
        | !reagentCountRead
        | !equippedItemClassRead
        | !equippedItemSubclassRead
        | !equippedItemInvTypesRead
        | !effectRead
        | !effectDieSidesRead
        | !effectRealPointsPerLevelRead
        | !effectBasePointsRead
        | !effectMechanicRead
        | !implicitTargetARead
        | !implicitTargetBRead
        | !effectRadiusIndexRead
        | !effectAuraRead
        | !effectAuraPeriodRead
        | !effectAmplitudeRead
        | !effectChainTargetsRead
        | !effectItemTypeRead
        | !effectMiscValueRead
        | !effectMiscValueBRead
        | !effectTriggerSpellRead
        | !effectPointsPerComboRead
        | !effectSpellClassMaskARead
        | !effectSpellClassMaskBRead
        | !effectSpellClassMaskCRead
        | !spellVisualIDRead
        | !spellIconIDRead
        | !activeIconIDRead
        | !spellPriorityRead
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
        | !nameSubtextRead0
        | !nameSubtextRead1
        | !nameSubtextRead2
        | !nameSubtextRead3
        | !nameSubtextRead4
        | !nameSubtextRead5
        | !nameSubtextRead6
        | !nameSubtextRead7
        | !nameSubtextRead8
        | !nameSubtextRead9
        | !nameSubtextRead10
        | !nameSubtextRead11
        | !nameSubtextRead12
        | !nameSubtextRead13
        | !nameSubtextRead14
        | !nameSubtextRead15
        | !nameSubtextFlagsRead
        | !descriptionRead0
        | !descriptionRead1
        | !descriptionRead2
        | !descriptionRead3
        | !descriptionRead4
        | !descriptionRead5
        | !descriptionRead6
        | !descriptionRead7
        | !descriptionRead8
        | !descriptionRead9
        | !descriptionRead10
        | !descriptionRead11
        | !descriptionRead12
        | !descriptionRead13
        | !descriptionRead14
        | !descriptionRead15
        | !descriptionFlagsRead
        | !auraDescriptionRead0
        | !auraDescriptionRead1
        | !auraDescriptionRead2
        | !auraDescriptionRead3
        | !auraDescriptionRead4
        | !auraDescriptionRead5
        | !auraDescriptionRead6
        | !auraDescriptionRead7
        | !auraDescriptionRead8
        | !auraDescriptionRead9
        | !auraDescriptionRead10
        | !auraDescriptionRead11
        | !auraDescriptionRead12
        | !auraDescriptionRead13
        | !auraDescriptionRead14
        | !auraDescriptionRead15
        | !auraDescriptionFlagsRead
        | !manaCostPctRead
        | !startRecoveryCategoryRead
        | !startRecoveryTimeRead
        | !maxTargetLevelRead
        | !spellClassSetRead
        | !spellClassMaskRead
        | !maxTargetsRead
        | !defenseTypeRead
        | !preventionTypeRead
        | !stanceBarOrderRead
        | !effectChainAmplitudeRead
        | !minFactionIDRead
        | !minReputationRead
        | !requiredAuraVisionRead
        | !requiredTotemCategoryIDRead
        | !requiredAreasIDRead
        | !schoolMaskRead
        | !runeCostIDRead
        | !spellMissileIDRead
        | !powerDisplayIDRead
        | !effectBonusCoefficientRead
        | !descriptionVariablesIDRead
        | !difficultyRead) {
        return false;
    }

    if (stringBuffer) {
        this->m_name = &stringBuffer[nameOffset[s_locale]];
        this->m_nameSubtext = &stringBuffer[nameSubtextOffset[s_locale]];
        this->m_description = &stringBuffer[descriptionOffset[s_locale]];
        this->m_auraDescription = &stringBuffer[auraDescriptionOffset[s_locale]];
    } else {
        this->m_name = "";
        this->m_nameSubtext = "";
        this->m_description = "";
        this->m_auraDescription = "";
    }

    return true;
}

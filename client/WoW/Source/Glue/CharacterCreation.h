#ifndef WOW_SOURCE_GLUE_CHARACTERCREATION_H
#define WOW_SOURCE_GLUE_CHARACTERCREATION_H

#include <cstdint>
#include "Engine/Source/Storm/H/stpl.h"
#include "WoW/Source/Game/NameGen.h"
#include "WoW/Source/Glue/Types.h"

class CCharacterComponent;
class ChrClassesRec;
class ComponentData;
class CSimpleModelFFX;
struct CharacterPreferences;

class CCharacterCreation {
    public:
    static CharacterPreferences* m_charPreferences[22][2];
    static CCharacterComponent* m_character;
    static CNameGen m_nameGen;
    static CSimpleModelFFX* m_charCustomizeFrame;
    static float m_charFacing;
    static TSGrowableArray<int32_t> m_races;
    static TSFixedArray<ChrClassesRec*> m_classes;
    static int32_t m_existingCharacterIndex;
    static int32_t m_selectedClassID;
    static uint32_t m_raceIndex;
    static uint32_t m_prevSkinIndex;
    static uint32_t m_prevFaceIndex;
    static uint32_t m_prevHairColorIndex;
    static uint32_t m_prevHairStyleIndex;
    static uint32_t m_prevFacialHairIndex;

    static void CalcClasses(int32_t raceID);
    static void CreateCharacter(const char* name);
    static void CycleCharCustomization(CHAR_CUSTOMIZATION_TYPE type, int32_t delta);
    static void Dress();
    static int32_t GetExistingClassIndex();
    static int32_t GetExistingRaceIndex();
    static int32_t GetRandomClassID();
    static void GetRandomRaceAndSex(ComponentData* data);
    static int32_t Idle(const void* data, void* param);
    static void InitCharacterComponents(ComponentData* data, bool randomize);
    static void Initialize();
    static bool IsClassAvailable(int32_t classID);
    static void OnHide();
    static void RandomizeCharCustomization();
    static void ResetCharCustomizeInfo();
    static void SetCharacterFacing(float facing);
    static void SetCharCustomizeModel(const char* modelName);
    static void SetSelectedClass(int32_t classID);
    static void SetSelectedRace(uint32_t raceIndex);
    static void SetSelectedSex(uint32_t sexID);
    static void SetToExistingCharacter(uint32_t index);
    static void Shutdown();
};

bool IsRaceClassValid(int32_t raceID, int32_t classID);
void CharacterCreateRegisterScriptFunctions();
void CharacterCreateUnregisterScriptFunctions();

#endif

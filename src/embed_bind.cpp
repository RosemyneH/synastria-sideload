#include "sym.h"

#include <new>

#include "Engine/Source/Base/OsTypes.h"
#include "Engine/Source/Base/PlatformTypes.h"
#include "Engine/Source/Base/CDataStore.h"
#include "Engine/Source/Frame/CScriptObject.h"
#include "Engine/Source/Frame/CSimpleModel.h"
#include "Engine/Source/FrameScript/FrameScript.h"
#include "Engine/Source/Os/System.h"
#include "Engine/Source/Os/TimeManager.h"
#include "Engine/Source/lua-5.1/src/lua.h"
#include "WoW/Source/DB/DBClient/DBClient.h"
#include "WoW/Source/DB/WowClientDB.h"
#include "WoW/Source/Glue/CharacterSelection.h"
#include "WoW/Source/Magic/MagicClient/Spell_C.h"
#include "WoW/Source/Net/NetClient/NetClient.h"
#include "WoW/Source/Object/MovementShared.h"
#include "WoW/Source/Object/ObjectClient/Bag_C.h"
#include "WoW/Source/Object/ObjectClient/Item_C.h"
#include "WoW/Source/Object/ObjectClient/Player_C.h"
#include "WoW/Source/Object/ObjectClient/Unit_C.h"
#include "WoW/Source/ObjectMgrClient/ObjectMgrClient.h"
#include "WoW/Source/UIUtil/Tooltip.h"
#include "WoW/Source/Ui/AchievementInfo.h"
#include "WoW/Source/Ui/ContainerFrame.h"
#include "WoW/Source/Ui/EquipmentManager.h"
#include "WoW/Source/Ui/GameUI.h"
#include "WoW/Source/Ui/GossipInfo.h"
#include "WoW/Source/Ui/MerchantFrame.h"
#include "WoW/Source/Ui/QuestFrame.h"
#include "WoW/Source/Ui/QuestLog.h"
#include "WoW/Source/Ui/QuestTextParser.h"
#include "WoW/Source/Ui/ReputationInfo.h"
#include "WoW/Source/Ui/SkillInfo.h"
#include "WoW/Source/Ui/SpellBookFrame.h"
#include "WoW/Source/Ui/TalentInfo.h"
#include "WoW/Source/Ui/TradeSkillFrame.h"
#include "WoW/Source/Ui/WorldMap.h"
#include "WoW/Source/WowServices/RealmConnection.h"
#include "WoW/Source/WowSvcs/WowSvcsClient/ClientServices.h"

namespace {

template <class T>
void bind_fn(const char* name, T fn) {
    syn_bind(name, reinterpret_cast<void*>(fn));
}

void bind_ptr(const char* name, const void* addr) {
    syn_bind(name, const_cast<void*>(addr));
}

void put_u8(void* self, uint8_t value) {
    static_cast<CDataStore*>(self)->Put(value);
}

void put_u16(void* self, uint16_t value) {
    static_cast<CDataStore*>(self)->Put(value);
}

void put_u32(void* self, uint32_t value) {
    static_cast<CDataStore*>(self)->Put(value);
}

void put_str(void* self, const char* text) {
    static_cast<CDataStore*>(self)->PutString(text);
}

void get_u8(void* self, uint8_t* out) {
    static_cast<CDataStore*>(self)->Get(*out);
}

void store_ctor(void* self) {
    new (self) CDataStore();
}

void store_dtor(void* self) {
    static_cast<CDataStore*>(self)->~CDataStore();
}

void store_finalize(void* self) {
    static_cast<CDataStore*>(self)->Finalize();
}

void net_set(void* self, int id, MESSAGE_HANDLER handler, void* param) {
    static_cast<NetClient*>(self)->SetMessageHandler(static_cast<NETMESSAGE>(id), handler, param);
}

void net_send(void* self, CDataStore* msg) {
    static_cast<NetClient*>(self)->Send(msg);
}

void* lookup_ability(void* self, int id) {
    return static_cast<CGUnit_C*>(self)->LookupAbility(id);
}

void feather(void* self) {
    static_cast<CMovementShared*>(self)->PostFeatherFall();
}

void* find_type(void* self, int id, unsigned flags) {
    return static_cast<CGBag_C*>(self)->FindItemOfType(id, flags);
}

bool find_totem(void* self, int category, unsigned flags) {
    return static_cast<CGBag_C*>(self)->FindTotemCategory(category, flags);
}

int type_count(void* self, int id, unsigned flags) {
    return static_cast<CGBag_C*>(self)->GetItemTypeCount(id, flags);
}

void* bag_find_item(void* self, int32_t (*callback)(const CGItem_C* item, void* param), void* param, WOWGUID& container, uint32_t& slot, uint32_t flags) {
    return static_cast<CGBag_C*>(self)->FindItem(callback, param, container, slot, flags);
}

const CGItem_C* bag_item(const void* self, int slot) {
    return static_cast<const CGBag_C*>(self)->GetItemPointer(slot);
}

const CGBag_C* player_bag(const void* self) {
    return static_cast<const CGPlayer_C*>(self)->GetBag();
}

int player_skill(const void* self, int id) {
    return static_cast<const CGPlayer_C*>(self)->GetSkillIndex(id);
}

bool item_bound(void* self) {
    return static_cast<CGItem_C*>(self)->IsBound();
}

C3Vector unit_position(const void* self) {
    return static_cast<const CGUnit_C*>(self)->GetPosition();
}

}  // namespace

void syn_bind_client() {
    bind_fn("_Z11CompressRowPKhjPh", &CompressRow);
    bind_fn("_Z13DecompressRowPKhjPh", &DecompressRow);
    bind_fn("_Z13GetBagAtIndexj", &GetBagAtIndex);
    bind_fn("_Z16GetSpellIdByNamePKcPi", &GetSpellIdByName);
    bind_fn("_Z16OsGetAsyncTimeMsv", &OsGetAsyncTimeMs);
    bind_fn("_Z18ClntObjMgrGetMapIDv", &ClntObjMgrGetMapID);
    bind_fn("_Z18GetRepListRepValuei", &GetRepListRepValue);
    bind_fn("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &ClntObjMgrObjectPtr);
    bind_fn("_Z19FrameScript_ExecutePKcS0_S0_", static_cast<void (*)(const char*, const char*, const char*)>(&FrameScript_Execute));
    bind_fn("_Z20SpellParserParseTextPK8SpellRecPcjiiiiii", &SpellParserParseText);
    bind_fn("_Z22FrameScript_GetContextv", &FrameScript_GetContext);
    bind_fn("_Z24Spell_C_GetSpellCooldowniiPjS_S_", &Spell_C_GetSpellCooldown);
    bind_fn("_Z25ClntObjMgrGetActivePlayerv", &ClntObjMgrGetActivePlayer);
    bind_fn("_Z25FrameScript_GetObjectThisP9lua_Statei", &FrameScript_GetObjectThis);
    bind_fn("_Z25OsValidateFunctionPointerPKv", &OsValidateFunctionPointer);
    bind_fn("_Z28ClntObjMgrGetActivePlayerObjv", &ClntObjMgrGetActivePlayerObj);
    bind_fn("_Z28FrameScript_RegisterFunctionPKcPFiP9lua_StateE", &FrameScript_RegisterFunction);
    bind_fn("_Z8HasSpellj", static_cast<bool (*)(uint32_t)>(&HasSpell));
    bind_fn("_ZN10CDataStore3PutEh", &put_u8);
    bind_fn("_ZN10CDataStore3PutEj", &put_u32);
    bind_fn("_ZN10CDataStore3PutEt", &put_u16);
    bind_fn("_ZN10CDataStore8FinalizeEv", &store_finalize);
    bind_fn("_ZN10CDataStore9PutStringEPKc", &put_str);
    bind_fn("_ZN10CDataStore3GetERh", &get_u8);
    bind_fn("_ZN10CDataStoreC1Ev", &store_ctor);
    bind_fn("_ZN10CDataStoreD1Ev", &store_dtor);
    bind_fn("_ZN10CGUnit_C13LookupAbilityEi", &lookup_ability);
    bind_ptr("_ZN10CGWorldMap12m_continentsE", &CGWorldMap::m_continents);
    bind_fn("_ZN10CGWorldMap16GetWorldPositionERK8C3VectoriPfS3_Pbbi", &CGWorldMap::GetWorldPosition);
    bind_ptr("_ZN10CGWorldMap18m_currentContinentE", &CGWorldMap::m_currentContinent);
    bind_ptr("_ZN11CGQuestInfo12m_questItemsE", &CGQuestInfo::m_questItems);
    bind_ptr("_ZN11CGQuestInfo14m_currentQuestE", &CGQuestInfo::m_currentQuest);
    bind_ptr("_ZN11CGSkillInfo10m_numLinesE", &CGSkillInfo::m_numLines);
    bind_ptr("_ZN11CGSkillInfo7m_linesE", &CGSkillInfo::m_lines);
    bind_ptr("_ZN12CGGossipInfo8m_questsE", &CGGossipInfo::m_quests);
    bind_fn("_ZN12CGTalentInfo10EnterWorldEv", &CGTalentInfo::EnterWorld);
    bind_fn("_ZN12CGTalentInfo10LeaveWorldEv", &CGTalentInfo::LeaveWorld);
    bind_fn("_ZN12CGTalentInfo16UpdateTalentDataEP10CDataStore", &CGTalentInfo::UpdateTalentData);
    bind_fn("_ZN12CSimpleModel13GetObjectTypeEv", &CSimpleModel::GetObjectType);
    bind_ptr("_ZN14CGMerchantInfo10m_merchantE", &CGMerchantInfo::m_merchant);
    bind_ptr("_ZN14CGMerchantInfo18m_numMerchantItemsE", &CGMerchantInfo::m_numMerchantItems);
    bind_ptr("_ZN14CGMerchantInfo7m_itemsE", &CGMerchantInfo::m_items);
    bind_ptr("_ZN14ClientServices15s_characterInfoE", &ClientServices::s_characterInfo);
    bind_ptr("_ZN14ClientServices19s_currentConnectionE", &ClientServices::s_currentConnection);
    bind_fn("_ZN14ClientServices17SetMessageHandlerE10NETMESSAGEPFiPvS0_jP10CDataStoreES1_", &ClientServices::SetMessageHandler);
    bind_fn("_ZN14ClientServices5Send2EP10CDataStore", &ClientServices::Send2);
    bind_fn("_ZN15CMovementShared15PostFeatherFallEv", &feather);
    bind_ptr("_ZN16CGReputationInfo18m_factionStandingsE", &CGReputationInfo::m_factionStandings);
    bind_fn("_ZN16CGTradeSkillInfo12DoTradeSkillEij", &CGTradeSkillInfo::DoTradeSkill);
    bind_ptr("_ZN16CGTradeSkillInfo15m_recastSpellIDE", &CGTradeSkillInfo::m_recastSpellID);
    bind_ptr("_ZN18CGEquipmentManager15s_equipmentSetsE", &CGEquipmentManager::s_equipmentSets);
    bind_ptr("_ZN19CCharacterSelection15s_characterListE", &CCharacterSelection::s_characterList);
    bind_fn("_ZN7CGBag_C14FindItemOfTypeEij", &find_type);
    bind_fn("_ZN7CGBag_C16GetItemTypeCountEij", &type_count);
    bind_fn("_ZN7CGBag_C17FindTotemCategoryEij", &find_totem);
    bind_fn("_ZN7CGBag_C8FindItemEPFiPK8CGItem_CPvES3_R7WOWGUIDRjj", &bag_find_item);
    bind_fn("_ZN7Spell_C9CastSpellEiPK8CGItem_C7WOWGUIDb", &Spell_C::CastSpell);
    bind_fn("_ZN8CGGameUI10UnlockItemE7WOWGUID", &CGGameUI::UnlockItem);
    bind_ptr("_ZN8CGGameUI11m_subzoneIDE", &CGGameUI::m_subzoneID);
    bind_ptr("_ZN8CGGameUI12m_cursorItemE", &CGGameUI::m_cursorItem);
    bind_fn("_ZN8CGGameUI13GetCursorItemEv", static_cast<WOWGUID (*)()>(&CGGameUI::GetCursorItem));
    bind_ptr("_ZN8CGGameUI15m_iCurrentMapIDE", &CGGameUI::m_iCurrentMapID);
    bind_ptr("_ZN8CGGameUI16m_cursorItemTypeE", &CGGameUI::m_cursorItemType);
    bind_fn("_ZN8CGGameUI8LockItemE7WOWGUID", &CGGameUI::LockItem);
    bind_ptr("_ZN8CGGameUI8m_areaIDE", &CGGameUI::m_areaID);
    bind_fn("_ZN8CGItem_C15GetInventoryArtEi", static_cast<const char* (*)(int32_t)>(&CGItem_C::GetInventoryArt));
    bind_fn("_ZN8CGItem_C7IsBoundEv", &item_bound);
    bind_fn("_ZN8CGUnit_C13LookupAbilityEi", &lookup_ability);
    bind_fn("_ZN9CGTooltip22GetItemHyperlinkStringEP8CGItem_C", static_cast<const char* (*)(CGItem_C*)>(&CGTooltip::GetItemHyperlinkString));
    bind_fn("_ZN9NetClient17SetMessageHandlerE10NETMESSAGEPFiPvS0_jP10CDataStoreES1_", &net_set);
    bind_fn("_ZN9NetClient4SendEP10CDataStore", &net_send);
    bind_fn("_ZNK10CGPlayer_C13GetSkillIndexEi", &player_skill);
    bind_fn("_ZNK10CGPlayer_C6GetBagEv", &player_bag);
    bind_fn("_ZNK7CGBag_C14GetItemPointerEi", &bag_item);
    bind_fn("_ZNK8CGUnit_C11GetPositionEv", &unit_position);
    bind_ptr("g_areaGroupDB", &g_areaGroupDB);
    bind_ptr("g_areaTableDB", &g_areaTableDB);
    bind_ptr("g_clientDbPerformCompress", &g_clientDbPerformCompress);
    bind_ptr("g_factionDB", &g_factionDB);
    bind_ptr("g_itemDB", &g_itemDB);
    bind_ptr("g_itemSubClassDB", &g_itemSubClassDB);
    bind_ptr("g_mapDB", &g_mapDB);
    bind_ptr("g_skillLineDB", &g_skillLineDB);
    bind_ptr("g_spellDB", &g_spellDB);
    bind_ptr("g_spellFocusObjectDB", &g_spellFocusObjectDB);
    bind_ptr("g_totemCategoryDB", &g_totemCategoryDB);
    bind_fn("lua_checkstack", &lua_checkstack);
    bind_fn("lua_createtable", &lua_createtable);
    bind_fn("lua_getfield", &lua_getfield);
    bind_fn("lua_isnumber", &lua_isnumber);
    bind_fn("lua_isstring", &lua_isstring);
    bind_fn("lua_pushboolean", &lua_pushboolean);
    bind_fn("lua_pushnil", &lua_pushnil);
    bind_fn("lua_pushnumber", &lua_pushnumber);
    bind_fn("lua_pushstring", &lua_pushstring);
    bind_fn("lua_rawset", &lua_rawset);
    bind_fn("lua_settable", &lua_settable);
    bind_fn("lua_settop", &lua_settop);
    bind_fn("lua_toboolean", &lua_toboolean);
    bind_fn("lua_tolstring", &lua_tolstring);
    bind_fn("lua_tonumber", &lua_tonumber);
    bind_fn("lua_type", &lua_type);
    bind_ptr("s_completedQuests", &s_completedQuests);
    bind_ptr("s_knownSpellBits", &s_knownSpellBits);
    bind_ptr("_ZN10CGQuestLog10m_numQuestE", &CGQuestLog::m_numQuest);
    bind_ptr("_ZN10CGQuestLog8m_questsE", &CGQuestLog::m_quests);
}

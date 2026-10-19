#ifndef WOW_SOURCE_DB_DBCLIENT_DBCACHEINSTANCES_H
#define WOW_SOURCE_DB_DBCLIENT_DBCACHEINSTANCES_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/DB/DBClient/CreatureStats_C.h"
#include "WoW/Source/DB/DBClient/DBCache.h"
#include "WoW/Source/DB/DBClient/GameObjectStats_C.h"
#include "WoW/Source/Game/GameClient/NameCache.h"
#include "WoW/Source/Game/GameClient/PetNameCache.h"
#include "WoW/Source/Game/GameClient/WardenClient.h"
#include "WoW/Source/Object/ArenaTeamCache.h"
#include "WoW/Source/Object/CGPetition.h"
#include "WoW/Source/Object/DanceCache.h"
#include "WoW/Source/Object/ItemNameCache.h"
#include "WoW/Source/Object/ItemStats.h"
#include "WoW/Source/Object/ItemTextCache.h"
#include "WoW/Source/Object/NPCText.h"
#include "WoW/Source/Object/ObjectClient/GuildStats_C.h"
#include "WoW/Source/Object/ObjectClient/Object_C.h"
#include "WoW/Source/Object/PageTextCache.h"
#include "WoW/Source/Object/QuestCache.h"

extern DBCache<CreatureStats_C, int32_t, HASHKEY_INT> g_creatureDBCache;
extern DBCache<GameObjectStats_C, int32_t, HASHKEY_INT> g_gameObjectDBCache;
extern DBCache<ItemNameCache_C, int32_t, HASHKEY_INT> g_itemNameDBCache;
extern DBCache<ItemStats_C, int32_t, HASHKEY_INT> g_itemDBCache;
extern DBCache<NPCText, int32_t, HASHKEY_INT> g_npcTextDBCache;
extern DBCache<NameCache, uint64_t, CHashKeyGUID> g_nameDBCache;
extern DBCache<GuildStats_C, int32_t, HASHKEY_INT> g_guildInfoCache;
extern DBCache<QuestCache, int32_t, HASHKEY_INT> g_questDBCache;
extern DBCache<PageTextCache_C, int32_t, HASHKEY_INT> g_pageTextCache;
extern DBCache<PetNameCache, int32_t, HASHKEY_INT> g_petNameCache;
extern DBCache<CGPetition, int32_t, HASHKEY_INT> g_petitionCache;
extern DBCache<ItemTextCache_C, uint64_t, CHashKeyGUID> g_itemTextDBCache;
extern DBCache<WardenCachedModule, CWardenKey, CWardenKey> g_wardenCache;
extern DBCache<ArenaTeamCache, int32_t, HASHKEY_INT> g_arenaTeamCache;
extern DBCache<DanceCache, int32_t, HASHKEY_INT> g_danceCache;

template <>
void DBCache<NameCache, uint64_t, CHashKeyGUID>::ItemAdded(NameCache* item);

template <>
void DBCache<NameCache, uint64_t, CHashKeyGUID>::Invalidate(uint64_t key);

template <>
DBCache<NameCache, uint64_t, CHashKeyGUID>::REVERSEENTRY* DBCache<NameCache, uint64_t, CHashKeyGUID>::FindReverseEntry(const char* name);

template <>
NameCache* DBCache<NameCache, uint64_t, CHashKeyGUID>::GetRecordByName(const char* name);

template <>
PetNameCache* DBCache<PetNameCache, int32_t, HASHKEY_INT>::GetRecordByName(const char* name);

template <>
void DBCache<NameCache, uint64_t, CHashKeyGUID>::RemoveReverseEntry(const char* name);

template <>
void DBCache<NameCache, uint64_t, CHashKeyGUID>::AddItem(NameCache* item, uint64_t key);

void DBCache_ClearHandlers();
void DBCache_ClearPending();
void DBCache_Destroy();
void DBCache_Idle();
void DBCache_Initialize();
void DBCache_RegisterHandlers();
void SetClientCacheVersion(uint32_t version);

#endif

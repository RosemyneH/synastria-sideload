#include "features.h"

#include "sym.h"

#include <lz4.h>

#include <cstdarg>
#include <cstdint>
#include <exception>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

constexpr uint32_t kMagic = 0x2838C53Au;
constexpr int kLuaGlobals = -10002;
constexpr int kMaxClasses = 10;

FILE* g_log = nullptr;

void log_line(const char* fmt, ...) {
    if (!g_log) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(g_log, fmt, args);
    va_end(args);
    fputc('\n', g_log);
    fflush(g_log);
}

struct ClassSlot {
    uint8_t id = 0;
    uint8_t extra = 0;
};

ClassSlot g_classes[kMaxClasses];
int g_class_count = 0;
int g_create_alt = 0;
bool g_classes_ready = false;
bool g_natives_ready = false;
bool g_scripts_ready = false;
bool g_in_world = false;
int g_script_wait = 0;

std::vector<std::string> g_scripts;

struct CacheRecord {
    uint32_t id;
    uint32_t size;
    uint8_t md5[16];
};

std::vector<CacheRecord> g_cache_records;
std::vector<std::string> g_item_name_list;
std::vector<std::string> g_obj_names;
std::vector<std::string> g_itemloc_names;
int g_obj_version = 0;
int g_itemloc_version = 0;
bool g_obj_loaded = false;
bool g_itemloc_loaded = false;

using RegisterFn = void (*)(const char*, int (*)(void*));
using ExecFn = void (*)(const char*, const char*, const char*);
using PushNumFn = void (*)(void*, double);
using PushStrFn = void (*)(void*, const char*);
using PushBoolFn = void (*)(void*, int);
using PushNilFn = void (*)(void*);
using ToNumFn = double (*)(void*, int);
using IsNumFn = int (*)(void*, int);

RegisterFn g_register = nullptr;
ExecFn g_exec = nullptr;
PushNumFn g_push_num = nullptr;
PushStrFn g_push_str = nullptr;
PushBoolFn g_push_bool = nullptr;
PushNilFn g_push_nil = nullptr;
ToNumFn g_to_num = nullptr;
IsNumFn g_is_num = nullptr;

struct ClassInfo {
    int id;
    const char* key;
    const char* name;
    int r, g, b;
};

const ClassInfo kClasses[] = {
    {1, "WARRIOR", "Warrior", 199, 156, 110},
    {2, "PALADIN", "Paladin", 245, 140, 186},
    {3, "HUNTER", "Hunter", 171, 212, 115},
    {4, "ROGUE", "Rogue", 255, 245, 105},
    {5, "PRIEST", "Priest", 255, 255, 255},
    {6, "DEATHKNIGHT", "Death Knight", 196, 31, 59},
    {7, "SHAMAN", "Shaman", 0, 112, 222},
    {8, "MAGE", "Mage", 64, 199, 235},
    {9, "WARLOCK", "Warlock", 135, 135, 237},
    {11, "DRUID", "Druid", 255, 125, 10},
};

bool valid_class(int id) {
    for (const ClassInfo& info : kClasses) {
        if (info.id == id) {
            return true;
        }
    }
    return false;
}

const ClassInfo* class_info(int id) {
    for (const ClassInfo& info : kClasses) {
        if (info.id == id) {
            return &info;
        }
    }
    return nullptr;
}

int arg_int(void* state, int index) {
    if (!g_is_num || !g_to_num || !g_is_num(state, index)) {
        return 0;
    }
    return static_cast<int>(g_to_num(state, index));
}

void push_name(void* state, int id) {
    const ClassInfo* info = class_info(id);
    if (!info) {
        g_push_nil(state);
        return;
    }
    g_push_str(state, info->name);
}

int cmc_count_real(void* state) {
    int count = 0;
    for (int i = 0; i < g_class_count; ++i) {
        if (g_classes[i].id) {
            ++count;
        }
    }
    if (count <= 0) {
        count = 1;
    }
    g_push_num(state, count);
    return 1;
}

int cmc_at(void* state) {
    int index = arg_int(state, 1) - 1;
    if (index < 0 || index >= g_class_count || !g_classes[index].id) {
        g_push_nil(state);
        return 1;
    }
    g_push_num(state, g_classes[index].id);
    return 1;
}

int cmc_has(void* state) {
    int id = arg_int(state, 1);
    int found = 0;
    for (int i = 0; i < g_class_count; ++i) {
        if (g_classes[i].id == id) {
            found = 1;
        }
    }
    g_push_bool(state, found);
    return 1;
}

int cmc_id(void* state) {
    g_push_num(state, g_class_count > 0 ? g_classes[0].id : 0);
    return 1;
}

int cmc_mask(void* state) {
    int mask = 0;
    for (int i = 0; i < g_class_count; ++i) {
        int id = g_classes[i].id;
        if (id > 0 && id <= 31) {
            mask |= 1 << (id - 1);
        }
    }
    g_push_num(state, mask);
    return 1;
}

int cmc_key(void* state) {
    const ClassInfo* info = class_info(arg_int(state, 1));
    if (!info) {
        g_push_nil(state);
        return 1;
    }
    g_push_str(state, info->key);
    return 1;
}

int cmc_name(void* state) {
    push_name(state, arg_int(state, 1));
    return 1;
}

int cmc_data(void* state) {
    int index = arg_int(state, 1) - 1;
    if (index < 0 || index >= g_class_count) {
        g_push_nil(state);
        return 1;
    }
    g_push_num(state, g_classes[index].id);
    g_push_num(state, g_classes[index].extra);
    return 2;
}

int cmc_enabled(void* state) {
    g_push_num(state, 0x82);
    return 1;
}

int cmc_dual_single(void* state) {
    int same = g_class_count >= 2 && g_classes[0].id && g_classes[0].id == g_classes[1].id;
    if (!same) {
        g_push_nil(state);
        return 1;
    }
    g_push_num(state, 1);
    return 1;
}

int class_rank(int id) {
    static const int kOrder[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
    for (int i = 0; i < 10; ++i) {
        if (kOrder[i] == id) {
            return i;
        }
    }
    return -1;
}

// ʕ •ᴥ•ʔ✿ Sorted class pairs from WoWExt.dll. Warrior+Druid is Barbarian. ✿ ʕ •ᴥ•ʔ
const char* combo_name(int a, int b) {
    static const char* kCombo[] = {
        "Warlord", "Champion", "Soldier", "Blademaster", "Crusader", "Butcher", "Ravager", "Battlemage", "Vanquisher", "Barbarian",
        "Justicar", "Avenger", "Agent", "Martyr", "Enforcer", "Sentinel", "Inquisitor", "Warden", "Guardian",
        "Windrunner", "Scout", "Seer", "Slayer", "Tempest", "Arcanist", "Trickster", "Ranger",
        "Shadowblade", "Herald", "Assassin", "Ninja", "Spellblade", "Voidstalker", "Spy",
        "Avatar", "Wraith", "Oracle", "Mystic", "Siren", "Empath",
        "Revenant", "Summoner", "Lich", "Necromancer", "Dreadweaver",
        "Stormrider", "Elementalist", "Harbinger", "Sage",
        "Wizard", "Sorcerer", "Conjurer",
        "Doomcaller", "Witch",
        "Wildspeaker",
    };
    int ra = class_rank(a);
    int rb = class_rank(b);
    if (ra < 0 || rb < 0) {
        return nullptr;
    }
    if (ra > rb) {
        int swap = ra;
        ra = rb;
        rb = swap;
    }
    int index = rb - ra;
    for (int i = 0; i < ra; ++i) {
        index += 10 - i;
    }
    if (index < 0 || index >= static_cast<int>(sizeof(kCombo) / sizeof(kCombo[0]))) {
        return nullptr;
    }
    return kCombo[index];
}

void push_pack_name(void* state, int pack) {
    int low = pack & 0xff;
    int high = (pack >> 8) & 0xff;
    const char* combo = combo_name(low, high);
    if (combo) {
        g_push_str(state, combo);
        return;
    }
    const ClassInfo* a = class_info(low);
    const ClassInfo* b = class_info(high);
    if (a) {
        g_push_str(state, a->name);
        return;
    }
    if (b) {
        g_push_str(state, b->name);
        return;
    }
    g_push_nil(state);
}

int cmc_from_pack(void* state) {
    push_pack_name(state, arg_int(state, 1));
    return 1;
}

int cmc_my_name(void* state) {
    if (g_class_count >= 2 && g_classes[0].id && g_classes[1].id) {
        const char* combo = combo_name(g_classes[0].id, g_classes[1].id);
        if (combo) {
            g_push_str(state, combo);
            return 1;
        }
        g_push_nil(state);
        return 1;
    }
    if (g_class_count >= 1 && g_classes[0].id) {
        push_name(state, g_classes[0].id);
        return 1;
    }
    g_push_nil(state);
    return 1;
}

void push_color(void* state, int id) {
    const ClassInfo* info = class_info(id);
    if (!info) {
        g_push_nil(state);
        return;
    }
    g_push_num(state, info->r / 255.0);
    g_push_num(state, info->g / 255.0);
    g_push_num(state, info->b / 255.0);
    char text[16];
    snprintf(text, sizeof(text), "|cff%02x%02x%02x", info->r, info->g, info->b);
    g_push_str(state, text);
}

int cmc_color(void* state) {
    int pack = arg_int(state, 1);
    int id = pack;
    if (!class_info(id)) {
        id = pack & 0xff;
        if (!class_info(id)) {
            id = (pack >> 8) & 0xff;
        }
    }
    if (!class_info(id)) {
        g_push_nil(state);
        return 1;
    }
    push_color(state, id);
    return 4;
}

int cmc_talents(void* state) {
    int id = arg_int(state, 1);
    int ok = 0;
    for (int i = 0; i < g_class_count; ++i) {
        if (g_classes[i].id == id) {
            ok = 1;
        }
    }
    g_push_bool(state, ok || g_class_count == 0);
    return 1;
}

int cmc_create_get(void* state) {
    g_push_num(state, g_create_alt);
    return 1;
}

int cmc_create_set(void* state) {
    g_create_alt = arg_int(state, 2);
    return 0;
}

int stub_native(void*) {
    return 0;
}

int native_obj_loaded(void* state) {
    g_push_bool(state, g_obj_loaded ? 1 : 0);
    return 1;
}

int native_obj_version(void* state) {
    g_push_num(state, g_obj_version);
    return 1;
}

int native_get_obj_loc(void* state) {
    int id = arg_int(state, 2);
    g_push_nil(state);
    if (id >= 1 && id <= static_cast<int>(g_obj_names.size())) {
        g_push_str(state, g_obj_names[static_cast<size_t>(id - 1)].c_str());
    } else {
        g_push_nil(state);
    }
    return 2;
}

int native_itemloc_loaded(void* state) {
    g_push_bool(state, g_itemloc_loaded ? 1 : 0);
    return 1;
}

int native_zero(void* state) {
    g_push_num(state, 0);
    return 1;
}

int native_true(void* state) {
    g_push_bool(state, 1);
    return 1;
}

int native_item_info(void* state) {
    g_push_nil(state);
    (void)g_item_name_list;
    return 1;
}

int native_item_id_from_name(void* state) {
    return 0;
}

void collect_cstr(const uint8_t* data, size_t size, size_t start, std::vector<std::string>& out) {
    size_t i = start;
    while (i < size) {
        if (data[i] < 32 || data[i] >= 127) {
            ++i;
            continue;
        }
        size_t j = i;
        while (j < size && data[j] >= 32 && data[j] < 127) {
            ++j;
        }
        if (j - i >= 4 && j < size && data[j] == 0) {
            out.emplace_back(reinterpret_cast<const char*>(data + i), j - i);
        }
        i = j + 1;
    }
}

uint8_t* read_alloc(const char* path, size_t* size) {
    *size = 0;
    FILE* file = fopen(path, "rb");
    if (!file) {
        return nullptr;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return nullptr;
    }
    long length = ftell(file);
    if (length <= 0) {
        fclose(file);
        return nullptr;
    }
    rewind(file);
    uint8_t* buf = static_cast<uint8_t*>(malloc(static_cast<size_t>(length)));
    if (!buf) {
        fclose(file);
        return nullptr;
    }
    size_t got = fread(buf, 1, static_cast<size_t>(length), file);
    fclose(file);
    if (got != static_cast<size_t>(length)) {
        free(buf);
        return nullptr;
    }
    *size = got;
    return buf;
}

void load_item_db(const uint8_t* data, size_t size) {
    log_line("WoWExt2.bin start %zu", size);
    if (size < 8) {
        return;
    }
    uint32_t version = 0;
    memcpy(&version, data, 4);
    if (version != 10) {
        log_line("WoWExt2.bin version %u rejected", version);
        return;
    }
    g_item_name_list.clear();
    int named = 0;
    for (size_t i = 4; i + 3 < size; ++i) {
        if (data[i] != 0x3b) {
            continue;
        }
        uint8_t length = data[i + 2];
        if (length == 0 || length > 80 || i + 3 + length > size) {
            continue;
        }
        bool printable = true;
        for (uint8_t n = 0; n < length; ++n) {
            unsigned char c = data[i + 3 + n];
            if (c < 32 || c >= 127) {
                printable = false;
                break;
            }
        }
        if (!printable) {
            continue;
        }
        g_item_name_list.emplace_back(reinterpret_cast<const char*>(data + i + 3), length);
        ++named;
        i += 2 + length;
    }
    log_line("WoWExt2.bin %zu bytes version %u named items %d", size, version, named);
}

void load_obj_loc(const uint8_t* data, size_t size) {
    if (size < 4) {
        return;
    }
    uint32_t version = 0;
    memcpy(&version, data, 4);
    g_obj_version = static_cast<int>(version);
    collect_cstr(data, size, 20, g_obj_names);
    g_obj_loaded = version == 1 && !g_obj_names.empty();
    log_line("WoWExtObjLoc.bin %zu bytes version %u names %zu", size, version, g_obj_names.size());
}

void load_item_loc(const uint8_t* data, size_t size) {
    if (size < 4) {
        return;
    }
    uint32_t version = 0;
    memcpy(&version, data, 4);
    g_itemloc_version = static_cast<int>(version);
    collect_cstr(data, size, 16, g_itemloc_names);
    g_itemloc_loaded = version == 5 && !g_itemloc_names.empty();
    log_line("WoWExtItemLoc.bin %zu bytes version %u names %zu", size, version, g_itemloc_names.size());
}

const char* kStubs[] = {
    "GetItemExtraCustom", "GetItemTagsCustom", "GetItemAffixMask",
    "Custom_GetLootFilter", "Custom_GetLootFilterRule", "Custom_GetLootFilterRuleEntry",
    "Custom_SetLootFilter", "Custom_SetLootFilterRule", "Custom_SetLootFilterRuleEntry",
    "Custom_SetLootFilterOptions", "Custom_SetLootFilterRuleEnabled", "Custom_LootFilterResetToDefault",
    "Custom_LootFilterExport", "Custom_LootFilterImport", "Custom_LootFilterExportRule", "Custom_LootFilterImportRule",
    "Custom_SetItemSelectFilter", "Custom_GetItemSelectCount", "Custom_GetItemSelect",
    "Custom_SetDressupFacing", "Custom_ClearDressupFacing", "Custom_GetTransmogItem", "Custom_TransmogExport",
    "Custom_GetTransmogEnchant", "Custom_GetItemVars", "Custom_HasAcquiredItem", "Custom_FixupTransmogId",
    "Custom_CacheHaveItems", "Custom_GetHaveItems", "Custom_IsHaveItem", "Custom_GetQuestItemId",
    "ItemLocGetSourceCount", "ItemLocGetSourceAt", "ItemLocSetSourceSort", "ItemLocGetSourceSort",
    "ItemLocGetObjCount", "ItemLocGetObjAt", "ItemLocSetObjSort", "ItemLocFixupObjId",
    "ItemLocItemIsInZone", "ItemLocItemIsInMultiZone", "ItemLocSourceIsInZone", "ItemLocGetAllItemsInZone",
    "ItemLocGetAllObjsInZone", "ItemLocGetSourcesCompacted",
    "Custom_GetPrequest", "Custom_GetZoneName", "Custom_ZoneContains",
    "Custom_GetCurrentZone", "Custom_GetCurrentZoneOur", "Custom_GetQuestName", "Custom_GetQuestData",
    "Custom_GetQuestReq", "Custom_GetQuestCanAccept", "Custom_GetQuestCompleted",
    "Custom_GetCurrentCursorItem", "Custom_GetMerchantItem", "Custom_IsContainerItemSoulbound",
    "Custom_IsItemSoulbound", "Custom_GetItemCount", "Custom_IsItemEquipMgr", "Custom_GetItemGuid",
    "Custom_GetItemLinkBySlot", "Custom_GetIdFromLink", "Custom_IsPlayerNear",
    "Custom_RegisterForUseContainerItem", "Custom_UnregisterForUseContainerItem",
    "Custom_Internal_SetResultFromUseContainerItem", "Custom_LockItem", "Custom_UnlockItem",
    "Custom_GetEntityExtraDataInt", "Custom_GetEntityExtraDataFloat",
    "Custom_SetEntityExtraDataInt", "Custom_SetEntityExtraDataFloat",
    "Custom_IsOnQuest", "Custom_HaveAnyRewardOfQuest", "Custom_GetQuestFaction",
    "Custom_SetTradeSkillCustomFilter", "Custom_GetSkillIndex",
    "Custom_GetSpellCooldown", "Custom_GetSpellDesc", "Custom_GetSpellTools", "Custom_GetSpellEffect",
    "Custom_GetProfessionRecipes", "Custom_GetProfessionRecipeInfo", "Custom_GetProfessionRecipeFromCraftedItem",
    "Custom_GetProfessionRecipeReagents", "Custom_DoProfessionRecipe", "Custom_GetTradeSkillTools",
    "Custom_GetGossipQuests", "Custom_GetHighestLearnedSpell", "Custom_TranslateMapPosition",
    "Custom_AddTrackObjLoc", "Custom_RemoveTrackObjLoc",
    "Custom_ClearTrackObjLoc", "Custom_CalculateMapMarker",
    "Custom_GetMapFilteredResults", "Custom_SendQuestTrack", "Custom_GetPendingEvents",
    "Custom_GetPendingEventsEx", "Custom_QueueEvent", "Custom_QueueEventEx",
};

bool bind_lua() {
    return syn_resolve("_Z28FrameScript_RegisterFunctionPKcPFiP9lua_StateE", reinterpret_cast<void**>(&g_register))
        && syn_resolve("_Z19FrameScript_ExecutePKcS0_S0_", reinterpret_cast<void**>(&g_exec))
        && syn_resolve("lua_pushnumber", reinterpret_cast<void**>(&g_push_num))
        && syn_resolve("lua_pushstring", reinterpret_cast<void**>(&g_push_str))
        && syn_resolve("lua_pushboolean", reinterpret_cast<void**>(&g_push_bool))
        && syn_resolve("lua_pushnil", reinterpret_cast<void**>(&g_push_nil))
        && syn_resolve("lua_tonumber", reinterpret_cast<void**>(&g_to_num))
        && syn_resolve("lua_isnumber", reinterpret_cast<void**>(&g_is_num));
}

struct ListedClass {
    uint8_t primary;
    uint8_t alt;
};

ListedClass g_listed[20]{};
void* g_list_data = nullptr;
int g_list_count = 0;
uint8_t* g_list_base = nullptr;

void harvest_characters() {
    if (!g_list_base) {
        void* symbol = nullptr;
        if (!syn_resolve("_ZN19CCharacterSelection15s_characterListE", &symbol)) {
            return;
        }
        g_list_base = static_cast<uint8_t*>(symbol);
    }
    int count = *reinterpret_cast<int*>(g_list_base + 4);
    uint8_t* data = *reinterpret_cast<uint8_t**>(g_list_base + 8);
    if (count < 0) {
        count = 0;
    }
    if (count > 20) {
        count = 20;
    }
    if (data != g_list_data) {
        memset(g_listed, 0, sizeof(g_listed));
        g_list_data = data;
    }
    g_list_count = count;
    if (!data) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        uint8_t* row = data + static_cast<size_t>(i) * 0x1a0;
        uint32_t* zone = reinterpret_cast<uint32_t*>(row + 0x3c);
        uint8_t alt = static_cast<uint8_t>(*zone >> 24);
        uint8_t primary = row[0x179];
        if (alt) {
            g_listed[i].primary = primary;
            g_listed[i].alt = alt;
            *zone &= 0x00ffffff;
        } else if (!g_listed[i].primary) {
            g_listed[i].primary = primary;
        }
    }
}

int native_harvest(void* state) {
    harvest_characters();
    (void)state;
    return 0;
}

int native_character_class(void* state) {
    harvest_characters();
    int index = arg_int(state, 1) - 1;
    if (index < 0 || index >= g_list_count || !g_listed[index].alt) {
        g_push_nil(state);
        return 1;
    }
    const char* name = combo_name(g_listed[index].primary, g_listed[index].alt);
    if (!name) {
        g_push_nil(state);
        return 1;
    }
    g_push_str(state, name);
    return 1;
}

void register_cmc(const char* name, int (*fn)(void*)) {
    g_register(name, fn);
}

void register_natives() {
    if (g_natives_ready || !g_register) {
        return;
    }
    register_cmc("CMCGetClassCount", cmc_count_real);
    register_cmc("CMCGetClassAt", cmc_at);
    register_cmc("CMCHasClass", cmc_has);
    register_cmc("CMCGetClassId", cmc_id);
    register_cmc("CMCGetClassMask", cmc_mask);
    register_cmc("CMCGetClassKey", cmc_key);
    register_cmc("CMCGetClassName", cmc_name);
    register_cmc("CMCGetClassData", cmc_data);
    register_cmc("CMCGetMultiClassEnabled", cmc_enabled);
    register_cmc("CMCIsDualSingleClass", cmc_dual_single);
    register_cmc("CMCGetClassNameFromPack", cmc_from_pack);
    register_cmc("CMCGetMyClassName", cmc_my_name);
    register_cmc("CMCGetClassNameColor", cmc_color);
    register_cmc("CMCSetSelectedTalents", cmc_talents);
    register_cmc("CMCCreateGetSelectedClass", cmc_create_get);
    register_cmc("CMCCreateSetSelectedClass", cmc_create_set);
    register_cmc("SynastriaHarvest", native_harvest);
    register_cmc("SynastriaCharacterClass", native_character_class);
    register_cmc("GetItemInfoCustom", native_item_info);
    register_cmc("Custom_GetItemIdFromName", native_item_id_from_name);
    register_cmc("Custom_IsObjLocLoaded", native_obj_loaded);
    register_cmc("Custom_GetObjLocVersion", native_obj_version);
    register_cmc("Custom_GetObjLoc", native_get_obj_loc);
    register_cmc("ItemLocIsLoaded", native_itemloc_loaded);
    register_cmc("GetCustomGameData", native_zero);
    register_cmc("GetCustomGameDataStr", stub_native);
    register_cmc("Custom_GetAllTrackObjLoc", stub_native);
    register_cmc("Custom_GetMapMarker", native_zero);
    register_cmc("Custom_GetTotalMapMarkerCount", native_zero);
    register_cmc("Custom_SetMapMarkerOption", native_zero);
    register_cmc("Custom_AddTrackObjLoc", native_true);
    for (const char* name : kStubs) {
        g_register(name, stub_native);
    }
    g_natives_ready = true;
    log_line("registered CMC natives and %zu custom stubs", sizeof(kStubs) / sizeof(kStubs[0]));
}

void run_scripts() {
    if (g_scripts_ready || !g_exec) {
        return;
    }
    if (g_scripts.empty()) {
        log_line("container scripts were empty");
        g_scripts_ready = true;
        return;
    }
    for (const std::string& source : g_scripts) {
        g_exec(source.c_str(), "WoWExt.bin", "synastria");
    }
    g_scripts_ready = true;
    log_line("executed %zu WoWExt.bin lua containers", g_scripts.size());
}

bool read_u32(const uint8_t* data, size_t size, size_t& offset, uint32_t& out) {
    if (offset + 4 > size) {
        return false;
    }
    memcpy(&out, data + offset, 4);
    offset += 4;
    return true;
}

void load_container(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        log_line("WoWExt.bin missing");
        return;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    std::vector<uint8_t> data(static_cast<size_t>(length));
    if (length <= 0 || fread(data.data(), 1, data.size(), file) != data.size()) {
        fclose(file);
        return;
    }
    fclose(file);

    size_t offset = 0;
    uint32_t magic = 0;
    uint32_t version = 0;
    uint32_t count = 0;
    if (!read_u32(data.data(), data.size(), offset, magic) || magic != kMagic
        || !read_u32(data.data(), data.size(), offset, version) || version != 1
        || !read_u32(data.data(), data.size(), offset, count) || count >= 0x1000) {
        log_line("WoWExt.bin header rejected");
        return;
    }

    int lua_count = 0;
    int other_count = 0;
    std::vector<char> plain(1 << 20);
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t id = 0;
        uint32_t data_size = 0;
        if (!read_u32(data.data(), data.size(), offset, id) || offset + 16 + 4 > data.size()) {
            break;
        }
        CacheRecord record{};
        record.id = id;
        memcpy(record.md5, data.data() + offset, 16);
        offset += 16;
        if (!read_u32(data.data(), data.size(), offset, data_size) || offset + data_size > data.size()) {
            break;
        }
        record.size = data_size;
        g_cache_records.push_back(record);
        const uint8_t* payload = data.data() + offset;
        offset += data_size;
        if (data_size == 0) {
            continue;
        }
        int plain_size = LZ4_decompress_safe(reinterpret_cast<const char*>(payload), plain.data(),
                                              static_cast<int>(data_size), static_cast<int>(plain.size()));
        if (plain_size < 8) {
            ++other_count;
            continue;
        }
        int kind = static_cast<int8_t>(plain[1]);
        if (kind != 2) {
            ++other_count;
            continue;
        }
        g_scripts.emplace_back(plain.data() + 8, plain.data() + plain_size);
        ++lua_count;
        (void)id;
    }
    log_line("WoWExt.bin containers lua=%d other=%d", lua_count, other_count);
}

void adopt_classes(const uint8_t* rest, uint32_t size) {
    if (size < 2 || rest[0] != 1) {
        return;
    }
    uint8_t count = rest[1];
    if (count == 0 || count >= kMaxClasses || size < 2u + count * 2u) {
        return;
    }
    ClassSlot next[kMaxClasses];
    for (uint8_t i = 0; i < count; ++i) {
        uint8_t id = rest[2 + i * 2];
        uint8_t extra = rest[3 + i * 2];
        if (!valid_class(id)) {
            return;
        }
        next[i] = {id, extra};
    }
    memcpy(g_classes, next, sizeof(next));
    g_class_count = count;
    g_classes_ready = true;
    log_line("dual class list count=%u first=%u second=%u", count, next[0].id, count > 1 ? next[1].id : 0);
}

}  // namespace

void syn_features_set_log(FILE* log) {
    g_log = log;
}

void syn_features_load(const char* root) {
    char path[1024];
    try {
        size_t size = 0;
        snprintf(path, sizeof(path), "%s/WoWExt2.bin", root);
        if (uint8_t* bytes = read_alloc(path, &size)) {
            load_item_db(bytes, size);
            free(bytes);
        } else {
            log_line("WoWExt2.bin missing");
        }
        snprintf(path, sizeof(path), "%s/WoWExtItemLoc.bin", root);
        if (uint8_t* bytes = read_alloc(path, &size)) {
            load_item_loc(bytes, size);
            free(bytes);
        } else {
            log_line("WoWExtItemLoc.bin missing");
        }
        snprintf(path, sizeof(path), "%s/WoWExtObjLoc.bin", root);
        if (uint8_t* bytes = read_alloc(path, &size)) {
            load_obj_loc(bytes, size);
            free(bytes);
        } else {
            log_line("WoWExtObjLoc.bin missing");
        }
        snprintf(path, sizeof(path), "%s/WoWExt3.bin", root);
        if (uint8_t* bytes = read_alloc(path, &size)) {
            log_line("WoWExt3.bin %zu bytes", size);
            free(bytes);
        } else {
            log_line("WoWExt3.bin missing");
        }
        snprintf(path, sizeof(path), "%s/WoWExt.bin", root);
        load_container(path);
    } catch (const std::exception& error) {
        log_line("bin load failed: %s", error.what());
    }
}

void syn_features_before() {
    if (!g_register && !bind_lua()) {
        return;
    }
    register_natives();
}

void syn_features_reset() {
    g_natives_ready = false;
    g_scripts_ready = false;
    g_classes_ready = false;
    g_script_wait = 0;
}

void syn_features_world() {
    g_in_world = true;
}

void syn_features_harvest() {
    harvest_characters();
}

void syn_features_after() {
    if (!g_natives_ready || g_scripts_ready) {
        return;
    }
    if (g_classes_ready || (g_in_world && g_script_wait++ > 180)) {
        run_scripts();
    }
}

void syn_features_packet(int cmd, const uint8_t* rest, uint32_t size) {
    if (cmd == 14 && rest) {
        adopt_classes(rest, size);
    }
}

int syn_features_cache_missing(const uint8_t* rest, uint32_t size, uint32_t* missing, int cap) {
    if (!rest || size < 5 || rest[0] != 0 || !missing || cap <= 0) {
        return -1;
    }
    uint32_t count = 0;
    memcpy(&count, rest + 1, 4);
    if (count > 0x1000 || 5u + count * 24u > size) {
        log_line("container sync rejected count=%u size=%u", count, size);
        return -1;
    }
    int need = 0;
    int matched = 0;
    const uint8_t* cursor = rest + 5;
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t id = 0;
        uint32_t bytes = 0;
        memcpy(&id, cursor, 4);
        memcpy(&bytes, cursor + 4, 4);
        const uint8_t* md5 = cursor + 8;
        cursor += 24;
        const CacheRecord* found = nullptr;
        for (const CacheRecord& record : g_cache_records) {
            if (record.id == id) {
                found = &record;
                break;
            }
        }
        if (found && found->size == bytes && memcmp(found->md5, md5, 16) == 0) {
            ++matched;
            continue;
        }
        if (need < cap) {
            missing[need] = id;
        }
        ++need;
    }
    log_line("container sync server=%u local=%zu matched=%d missing=%d", count, g_cache_records.size(), matched, need);
    return need < cap ? need : cap;
}

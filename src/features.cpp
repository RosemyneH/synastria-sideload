#include "features.h"

#include "sym.h"

#include <lz4.h>

#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <exception>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <map>
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

struct LuaScript {
    int priority = 0;
    uint32_t id = 0;
    uint32_t group = 0;
    std::string source;
};

std::vector<LuaScript> g_scripts;

struct GameCat {
    std::map<int, double> nums;
    std::map<int, std::string> strs;
};

std::map<int, GameCat> g_game;

void set_num(int cat, int key, double value) {
    g_game[cat].nums[key] = value;
}

GameCat* game_cat(int cat) {
    auto found = g_game.find(cat);
    if (found == g_game.end()) {
        return nullptr;
    }
    return &found->second;
}

struct CacheRecord {
    uint32_t id;
    uint32_t size;
    uint8_t md5[16];
};

std::vector<CacheRecord> g_cache_records;
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
using IsStrFn = int (*)(void*, int);
using ToStrFn = const char* (*)(void*, int, size_t*);
using GetFieldFn = void (*)(void*, int, const char*);
using SetTopFn = void (*)(void*, int);
using TypeFn = int (*)(void*, int);
using ContextFn = void* (*)();

RegisterFn g_register = nullptr;
ExecFn g_exec = nullptr;
PushNumFn g_push_num = nullptr;
PushStrFn g_push_str = nullptr;
PushBoolFn g_push_bool = nullptr;
PushNilFn g_push_nil = nullptr;
ToNumFn g_to_num = nullptr;
IsNumFn g_is_num = nullptr;
IsStrFn g_is_str = nullptr;
ToStrFn g_to_str = nullptr;
GetFieldFn g_get_field = nullptr;
SetTopFn g_set_top = nullptr;
TypeFn g_type = nullptr;
ContextFn g_context = nullptr;

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

int native_empty(void* state) {
    g_push_str(state, "");
    return 1;
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

struct ItemRow {
    bool on = false;
    uint8_t klass = 0;
    uint8_t quality = 0;
    uint8_t sub = 0;
    uint8_t inv = 0;
    uint32_t display = 0;
    uint32_t level = 0;
    std::string name;
};

std::vector<ItemRow> g_items;
std::vector<uint8_t*> g_item_blobs;
char g_root[1024] = {};
bool g_items_loaded = false;

using ArtFn = const char* (*)(int);
ArtFn g_art = nullptr;

const char* item_class_name(int klass) {
    static const char* names[] = {
        "Consumable", "Container", "Weapon", "Gem", "Armor", "Reagent",
        "Projectile", "Trade Goods", "", "Recipe", "", "Quiver",
        "Quest", "Key", "", "Miscellaneous", "Glyph",
    };
    if (klass < 0 || klass >= static_cast<int>(sizeof(names) / sizeof(names[0]))) {
        return "";
    }
    return names[klass];
}

const char* item_subclass_name(int klass, int sub) {
    if (klass == 2) {
        static const char* names[] = {
            "Axe", "Axe", "Bow", "Gun", "Mace", "Mace", "Polearm", "Sword", "Sword",
            "", "Staff", "", "", "Fist Weapon", "", "Dagger", "Thrown", "", "Crossbow", "Wand", "Fishing Pole",
        };
        if (sub >= 0 && sub < static_cast<int>(sizeof(names) / sizeof(names[0])) && names[sub][0]) {
            return names[sub];
        }
    }
    if (klass == 4) {
        static const char* names[] = {"Miscellaneous", "Cloth", "Leather", "Mail", "Plate", "", "Shield"};
        if (sub >= 0 && sub < static_cast<int>(sizeof(names) / sizeof(names[0])) && names[sub][0]) {
            return names[sub];
        }
    }
    return "";
}

const char* inv_name(int inv) {
    static const char* names[] = {
        "", "INVTYPE_HEAD", "INVTYPE_NECK", "INVTYPE_SHOULDER", "INVTYPE_BODY",
        "INVTYPE_CHEST", "INVTYPE_WAIST", "INVTYPE_LEGS", "INVTYPE_FEET", "INVTYPE_WRIST",
        "INVTYPE_HAND", "INVTYPE_FINGER", "INVTYPE_TRINKET", "INVTYPE_WEAPON", "INVTYPE_SHIELD",
        "INVTYPE_RANGED", "INVTYPE_CLOAK", "INVTYPE_2HWEAPON", "INVTYPE_BAG", "INVTYPE_TABARD",
        "INVTYPE_ROBE", "INVTYPE_WEAPONMAINHAND", "INVTYPE_WEAPONOFFHAND", "INVTYPE_HOLDABLE",
        "INVTYPE_AMMO", "INVTYPE_THROWN", "INVTYPE_RANGEDRIGHT", "INVTYPE_QUIVER", "INVTYPE_RELIC",
    };
    if (inv < 0 || inv >= static_cast<int>(sizeof(names) / sizeof(names[0]))) {
        return "";
    }
    return names[inv];
}

const char* quality_color(int quality) {
    static const char* colors[] = {
        "9d9d9d", "ffffff", "1eff00", "0070dd", "a335ee", "ff8000", "e6cc80", "00ccff",
    };
    if (quality < 0 || quality > 7) {
        quality = 1;
    }
    return colors[quality];
}

const char* item_icon(int display) {
    if (display <= 0) {
        return nullptr;
    }
    if (!g_art) {
        syn_resolve("_ZN8CGItem_C15GetInventoryArtEi", reinterpret_cast<void**>(&g_art));
    }
    if (!g_art) {
        return nullptr;
    }
    const char* path = g_art(display);
    if (!path || !path[0]) {
        return nullptr;
    }
    return path;
}

int native_item_info(void* state) {
    int id = arg_int(state, 1);
    if (id < 0 || id >= static_cast<int>(g_items.size()) || !g_items[static_cast<size_t>(id)].on) {
        g_push_nil(state);
        return 1;
    }
    const ItemRow& row = g_items[static_cast<size_t>(id)];
    char link[320];
    snprintf(link, sizeof(link), "|cff%s|Hitem:%d:0:0:0:0:0:0:0:80|h[%s]|h|r",
             quality_color(row.quality), id, row.name.c_str());
    g_push_str(state, row.name.c_str());
    g_push_str(state, link);
    g_push_num(state, row.quality);
    g_push_num(state, row.level);
    g_push_num(state, 0);
    g_push_str(state, item_class_name(row.klass));
    g_push_str(state, item_subclass_name(row.klass, row.sub));
    g_push_num(state, 1);
    g_push_str(state, inv_name(row.inv));
    const char* icon = item_icon(static_cast<int>(row.display));
    if (icon) {
        g_push_str(state, icon);
    } else {
        g_push_nil(state);
    }
    g_push_num(state, 0);
    return 11;
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

struct BinReader {
    const uint8_t* data;
    size_t size;
    size_t off = 0;
    bool ok = true;

    bool take(void* dst, size_t n) {
        if (!ok || off + n > size) {
            ok = false;
            return false;
        }
        memcpy(dst, data + off, n);
        off += n;
        return true;
    }

    uint8_t u8() {
        uint8_t value = 0;
        take(&value, 1);
        return value;
    }

    uint16_t u16() {
        uint16_t value = 0;
        take(&value, 2);
        return value;
    }

    uint32_t u32() {
        uint32_t value = 0;
        take(&value, 4);
        return value;
    }
};

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

    for (uint8_t* blob : g_item_blobs) {
        free(blob);
    }
    g_item_blobs.clear();
    g_items.assign(65536, {});

    BinReader in{data, size, 4, true};
    int named = 0;
    for (int id = 0; id < 65536 && in.ok; ++id) {
        uint8_t head = in.u8();
        if (!in.ok || (head & 1) == 0) {
            continue;
        }
        uint8_t mask = in.u8();
        uint8_t length = in.u8();
        if (!in.ok || in.off + length > in.size) {
            in.ok = false;
            break;
        }
        ItemRow row;
        row.on = true;
        row.name.assign(reinterpret_cast<const char*>(in.data + in.off), length);
        in.off += length;
        row.klass = in.u8();
        row.quality = in.u8();
        row.sub = (head & 0x02) ? in.u8() : 0;
        if (head & 0x04) {
            in.u8();
        }
        if (head & 0x08) {
            in.u8();
        }
        row.inv = (head & 0x10) ? in.u8() : 0;
        if (head & 0x20) {
            in.u8();
        }
        if (head & 0x40) {
            in.u16();
        }
        if (head & 0x80) {
            in.u16();
        }
        row.level = (mask & 0x01) ? in.u8() : in.u16();
        if (mask & 0x02) {
            in.u8();
        }
        if (mask & 0x04) {
            in.u8();
        }
        if (mask & 0x08) {
            in.u8();
            in.u32();
        }
        row.display = (mask & 0x10) ? in.u16() : in.u32();
        in.u32();
        if (mask & 0x20) {
            in.u32();
            in.u32();
        }
        if (!in.ok) {
            break;
        }
        g_items[static_cast<size_t>(id)] = std::move(row);
        ++named;
    }
    log_line("WoWExt2.bin %zu bytes version %u items %d offset %zu", size, version, named, in.off);
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
    "Custom_GetPrequest", "Custom_ZoneContains",
    "Custom_GetCurrentZone", "Custom_GetCurrentZoneOur", "Custom_GetQuestData",
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
    "Custom_GetSpellCooldown", "Custom_GetSpellTools", "Custom_GetSpellEffect",
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
        && syn_resolve("lua_isnumber", reinterpret_cast<void**>(&g_is_num))
        && syn_resolve("lua_isstring", reinterpret_cast<void**>(&g_is_str))
        && syn_resolve("lua_tolstring", reinterpret_cast<void**>(&g_to_str))
        && syn_resolve("lua_getfield", reinterpret_cast<void**>(&g_get_field))
        && syn_resolve("lua_settop", reinterpret_cast<void**>(&g_set_top))
        && syn_resolve("lua_type", reinterpret_cast<void**>(&g_type))
        && syn_resolve("_Z22FrameScript_GetContextv", reinterpret_cast<void**>(&g_context));
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

int native_game_data(void* state) {
    GameCat* cat = game_cat(arg_int(state, 1));
    if (!cat) {
        g_push_num(state, 0);
        return 1;
    }
    auto found = cat->nums.find(arg_int(state, 2));
    g_push_num(state, found == cat->nums.end() ? 0 : found->second);
    return 1;
}

int native_game_string(void* state) {
    GameCat* cat = game_cat(arg_int(state, 1));
    if (!cat) {
        g_push_nil(state);
        return 1;
    }
    auto found = cat->strs.find(arg_int(state, 2));
    if (found == cat->strs.end()) {
        g_push_nil(state);
        return 1;
    }
    g_push_str(state, found->second.c_str());
    return 1;
}

int native_game_count(void* state) {
    GameCat* cat = game_cat(arg_int(state, 1));
    g_push_num(state, cat ? static_cast<double>(cat->nums.size()) : 0);
    return 1;
}

int native_game_index(void* state) {
    GameCat* cat = game_cat(arg_int(state, 1));
    int index = arg_int(state, 2) - 1;
    if (!cat || index < 0 || index >= static_cast<int>(cat->nums.size())) {
        g_push_nil(state);
        return 1;
    }
    auto it = cat->nums.begin();
    std::advance(it, index);
    g_push_num(state, it->first);
    return 1;
}

int native_set_game_data(void* state) {
    if (!g_is_num || !g_is_num(state, 3)) {
        return 0;
    }
    set_num(arg_int(state, 1), arg_int(state, 2), g_to_num(state, 3));
    return 0;
}

int native_set_game_string(void* state) {
    const char* text = g_to_str && g_is_str && g_is_str(state, 3) ? g_to_str(state, 3, nullptr) : "";
    g_game[arg_int(state, 1)].strs[arg_int(state, 2)] = text ? text : "";
    return 0;
}

int native_notify(void* state) {
    const char* text = g_to_str && g_is_str && g_is_str(state, 3) ? g_to_str(state, 3, nullptr) : nullptr;
    syn_net_notify(static_cast<uint32_t>(arg_int(state, 1)), static_cast<uint32_t>(arg_int(state, 2)), text);
    return 0;
}

int native_safe_invoke(void* state) {
    const char* name = g_to_str ? g_to_str(state, 1, nullptr) : nullptr;
    if (!name || !name[0] || !g_exec) {
        return 0;
    }
    std::string script = "local fn = _G[\"";
    for (const char* p = name; *p; ++p) {
        if (*p == '\\' || *p == '"') {
            script.push_back('\\');
        }
        script.push_back(*p);
    }
    script += "\"]\nif type(fn) == \"function\" then fn() end\n";
    g_exec(script.c_str(), "SynastriaSafeInvoke", "synastria");
    return 0;
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
    register_cmc("GetCustomGameData", native_game_data);
    register_cmc("GetCustomGameDataString", native_game_string);
    register_cmc("GetCustomGameDataCount", native_game_count);
    register_cmc("GetCustomGameDataIndex", native_game_index);
    register_cmc("SetCustomGameData", native_set_game_data);
    register_cmc("SetCustomGameDataString", native_set_game_string);
    register_cmc("NotifyServer", native_notify);
    register_cmc("SynastriaSafeInvoke", native_safe_invoke);
    register_cmc("GetAffixNameCustom", native_empty);
    register_cmc("Custom_GetZoneName", native_empty);
    register_cmc("Custom_GetQuestName", native_empty);
    register_cmc("Custom_GetSpellDesc", native_empty);
    register_cmc("GetAttuneOverwrite", stub_native);
    register_cmc("GetHighestAttunePct", native_zero);
    register_cmc("GetInProgressAttuneKeys", stub_native);
    register_cmc("GetSafeInvokeQueue", stub_native);
    register_cmc("CalculateAttunableCount", native_zero);
    register_cmc("CalculateAttunableAffixCount", native_zero);
    register_cmc("CalculateAttunedCount", native_zero);
    register_cmc("CalculateAttunedAffixCount", native_zero);
    register_cmc("CalculateAttunedCountHigh", native_zero);
    register_cmc("CanAttuneItemHelperEx", native_zero);
    register_cmc("HasAttunedAnyVariantEx", native_zero);
    register_cmc("IsAttunableBySomeone", native_zero);
    register_cmc("SetItemTrackerClassMask", native_zero);
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

bool frame_present(const char* name) {
    if (!g_context || !g_get_field || !g_type || !g_set_top) {
        return true;
    }
    void* state = g_context();
    if (!state) {
        return false;
    }
    g_get_field(state, kLuaGlobals, name);
    int kind = g_type(state, -1);
    g_set_top(state, -2);
    return kind != 0;
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
    int errors = 0;
    for (const LuaScript& script : g_scripts) {
        std::string wrapped = "SynastriaScriptError = nil\nlocal ok, err = pcall(function()\n";
        wrapped += script.source;
        wrapped += "\nend)\nif not ok then SynastriaScriptError = tostring(err) end\n";
        g_exec(wrapped.c_str(), "WoWExt.bin", "synastria");
        if (!g_context || !g_get_field || !g_to_str || !g_set_top) {
            continue;
        }
        void* state = g_context();
        if (!state) {
            continue;
        }
        g_get_field(state, kLuaGlobals, "SynastriaScriptError");
        const char* err = g_to_str(state, -1, nullptr);
        if (err && err[0]) {
            ++errors;
            if (errors <= 8) {
                log_line("lua %u error: %s", script.id, err);
            }
        }
        g_set_top(state, -2);
    }
    if (!frame_present("PerkMgrFrame") && !frame_present("Custom_CreateFrame")) {
        log_line("in-game lua did not install, will retry");
        return;
    }
    g_scripts_ready = true;
    log_line("executed %zu WoWExt.bin lua containers errors=%d", g_scripts.size(), errors);
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
        size_t cursor = 1;
        int type = 0;
        int shift = 0;
        bool typed = false;
        while (cursor < static_cast<size_t>(plain_size) && shift < 28) {
            uint8_t bite = plain[cursor++];
            type |= (bite & 0x7f) << shift;
            if ((bite & 0x80) == 0) {
                typed = true;
                break;
            }
            shift += 7;
        }
        if (!typed || type != 2 || cursor + 6 > static_cast<size_t>(plain_size)) {
            ++other_count;
            continue;
        }
        uint32_t group = 0;
        memcpy(&group, plain.data() + cursor, 4);
        cursor += 4;
        uint16_t slen = 0;
        memcpy(&slen, plain.data() + cursor, 2);
        cursor += 2;
        if (cursor + slen > static_cast<size_t>(plain_size)) {
            ++other_count;
            continue;
        }
        LuaScript script;
        script.priority = static_cast<int8_t>(plain[0]);
        script.id = id;
        script.group = group;
        script.source.assign(plain.data() + cursor, plain.data() + cursor + slen);
        g_scripts.push_back(std::move(script));
        ++lua_count;
    }
    // ʕ •ᴥ•ʔ✿ Priority byte, then id. A nonzero group is one script split across chunks that share locals. ✿ ʕ •ᴥ•ʔ
    std::sort(g_scripts.begin(), g_scripts.end(), [](const LuaScript& a, const LuaScript& b) {
        if (a.priority != b.priority) {
            return a.priority < b.priority;
        }
        return a.id < b.id;
    });
    std::map<uint32_t, size_t> bundle_at;
    std::vector<LuaScript> folded;
    folded.reserve(g_scripts.size());
    for (LuaScript& script : g_scripts) {
        if (script.group == 0) {
            folded.push_back(std::move(script));
            continue;
        }
        auto found = bundle_at.find(script.group);
        if (found == bundle_at.end()) {
            bundle_at.emplace(script.group, folded.size());
            folded.push_back(std::move(script));
            continue;
        }
        folded[found->second].source.push_back('\n');
        folded[found->second].source += script.source;
    }
    int bundles = lua_count - static_cast<int>(folded.size());
    g_scripts = std::move(folded);
    log_line("WoWExt.bin containers lua=%zu joined=%d other=%d", g_scripts.size(), bundles, other_count);
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

void syn_features_prepare(const char* root) {
    if (!root) {
        return;
    }
    snprintf(g_root, sizeof(g_root), "%s", root);
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
    // ʕ •ᴥ•ʔ✿ The constructor runs before these vectors exist, so the file load waits for idle. ✿ ʕ •ᴥ•ʔ
    if (!g_items_loaded && g_root[0]) {
        g_items_loaded = true;
        syn_features_load(g_root);
    }
    if (!g_register && !bind_lua()) {
        return;
    }
    register_natives();
}

void* syn_features_item_record(int id) {
    if (id < 0 || id >= static_cast<int>(g_items.size()) || !g_items[static_cast<size_t>(id)].on) {
        return nullptr;
    }
    if (g_item_blobs.size() != g_items.size()) {
        g_item_blobs.assign(g_items.size(), nullptr);
    }
    uint8_t*& blob = g_item_blobs[static_cast<size_t>(id)];
    if (blob) {
        return blob;
    }
    const ItemRow& row = g_items[static_cast<size_t>(id)];
    blob = static_cast<uint8_t*>(calloc(1, 0x280));
    if (!blob) {
        return nullptr;
    }
    auto put32 = [&](int at, uint32_t value) {
        memcpy(blob + at, &value, 4);
    };
    put32(0x08, row.klass);
    put32(0x0c, row.sub);
    put32(0x14, row.display);
    put32(0x18, row.quality);
    put32(0x2c, row.inv);
    put32(0x38, row.level);
    put32(0x60, 1);
    const char* name = row.name.c_str();
    memcpy(blob + 0x200, &name, sizeof(name));
    return blob;
}

void syn_features_reset() {
    g_natives_ready = false;
    g_scripts_ready = false;
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
    if (!g_classes_ready && !(g_in_world && g_script_wait > 30)) {
        if (g_in_world) {
            ++g_script_wait;
        }
        return;
    }
    if (g_script_wait > 0 && (g_script_wait % 45) != 0) {
        ++g_script_wait;
        return;
    }
    ++g_script_wait;
    run_scripts();
}

struct PktReader {
    const uint8_t* data;
    uint32_t size;
    uint32_t off = 0;
    bool ok = true;

    bool take(void* dst, uint32_t n) {
        if (!ok || off + n > size) {
            ok = false;
            return false;
        }
        memcpy(dst, data + off, n);
        off += n;
        return true;
    }

    uint8_t u8() {
        uint8_t value = 0;
        take(&value, 1);
        return value;
    }

    uint16_t u16() {
        uint16_t value = 0;
        take(&value, 2);
        return value;
    }

    uint32_t u32() {
        uint32_t value = 0;
        take(&value, 4);
        return value;
    }

    float f32() {
        float value = 0;
        take(&value, 4);
        return value;
    }
};

void apply_game_data(const uint8_t* rest, uint32_t size) {
    PktReader in{rest, size};
    uint16_t count = in.u16();
    int applied = 0;
    int skipped = 0;
    for (uint16_t i = 0; i < count && in.ok; ++i) {
        uint8_t kind = in.u8();
        if (!in.ok) {
            break;
        }
        if (kind == 4) {
            continue;
        }
        if (kind == 5) {
            int cat = static_cast<int>(in.u32());
            int key = static_cast<int>(in.u32());
            float value = in.f32();
            if (in.ok) {
                set_num(cat, key, value);
                ++applied;
            }
            continue;
        }
        if (kind == 6) {
            int cat = static_cast<int8_t>(in.u8());
            int key = in.u16();
            int raw = static_cast<int16_t>(in.u16());
            if (in.ok) {
                set_num(cat, key, raw * 0.01);
                ++applied;
            }
            continue;
        }
        if (kind == 7) {
            in.u32();
            continue;
        }
        if (kind == 0x16) {
            int cat = static_cast<int>(in.u32());
            uint32_t nstr = in.u32();
            for (uint32_t s = 0; s < nstr && in.ok; ++s) {
                int key = static_cast<int>(in.u32());
                uint8_t len = in.u8();
                if (!in.ok) {
                    break;
                }
                if (len == 0) {
                    g_game[cat].strs[key].clear();
                    continue;
                }
                if (in.off + len > in.size) {
                    in.ok = false;
                    break;
                }
                g_game[cat].strs[key].assign(reinterpret_cast<const char*>(in.data + in.off), len);
                in.off += len;
            }
            if (in.ok) {
                ++applied;
            }
            continue;
        }
        if (kind == 0x0b) {
            log_line("game data snapshot left unread");
            break;
        }
        in.u32();
        uint32_t bytes = in.u32();
        if (!in.ok || in.off + bytes > in.size) {
            in.ok = false;
            break;
        }
        in.off += bytes;
        ++skipped;
    }
    log_line("game data records=%u applied=%d blobs=%d", count, applied, skipped);
}

void syn_features_packet(int cmd, const uint8_t* rest, uint32_t size) {
    if (cmd == 13 && rest) {
        apply_game_data(rest, size);
    }
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

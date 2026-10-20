#include "features.h"

#include "sym.h"

#if defined(SYNASTRIA_EMBED)
#include "WoW/Source/Magic/MagicClient/SpellTable.h"
#include "WoW/Source/Ui/SpellBookFrame.h"
#endif

#include <lz4.h>

#include <algorithm>
#include <ctime>
#include <cstdarg>
#include <cstdint>
#include <exception>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#include <cmath>
#include <intrin.h>
inline int syn_ctzll(unsigned long long value) {
    unsigned long index = 0;
    _BitScanForward64(&index, value);
    return static_cast<int>(index);
}
inline int syn_clz(unsigned int value) {
    unsigned long index = 0;
    _BitScanReverse(&index, value);
    return 31 - static_cast<int>(index);
}
#define __builtin_ctzll syn_ctzll
#define __builtin_clz syn_clz
#define __builtin_popcount __popcnt
#define __builtin_round std::round
#endif

namespace {

std::string g_perk_lua;
std::string g_synergy_lua;
int g_perk_count = 0;
int g_perk_set_count = 0;
int g_synergy_count = 0;

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
std::vector<uint8_t> g_class_talents[kMaxClasses];
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
};

const ClassInfo kClasses[] = {
    {1, "WARRIOR", "Warrior"},
    {2, "PALADIN", "Paladin"},
    {3, "HUNTER", "Hunter"},
    {4, "ROGUE", "Rogue"},
    {5, "PRIEST", "Priest"},
    {6, "DEATHKNIGHT", "Death Knight"},
    {7, "SHAMAN", "Shaman"},
    {8, "MAGE", "Mage"},
    {9, "WARLOCK", "Warlock"},
    {11, "DRUID", "Druid"},
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

float bits_float(uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

struct Swatch {
    int id;
    uint32_t r, g, b;
};

// ʕ •ᴥ•ʔ✿ Raid swatches are the nameplate class colors. Pure swatches are the doubled-class mix. ✿ ʕ •ᴥ•ʔ
const Swatch kRaid[] = {
    {1, 0x3f47ae14u, 0x3f1c28f6u, 0x3edc28f6u},
    {2, 0x3f75c28fu, 0x3f0ccccdu, 0x3f3ae148u},
    {3, 0x3f2b851fu, 0x3f547ae1u, 0x3ee66666u},
    {4, 0x3f800000u, 0x3f75c28fu, 0x3ed1eb85u},
    {5, 0x3f800000u, 0x3f800000u, 0x3f800000u},
    {6, 0x3f451eb8u, 0x3df5c28fu, 0x3e6b851fu},
    {7, 0x00000000u, 0x3ee147aeu, 0x3f5eb852u},
    {8, 0x3ed1eb85u, 0x3f4ccccdu, 0x3f70a3d7u},
    {9, 0x3f147ae1u, 0x3f028f5cu, 0x3f4a3d71u},
    {11, 0x3f800000u, 0x3efae148u, 0x3d23d70au},
};

const Swatch kPure[] = {
    {1, 0x3f800000u, 0x00000000u, 0x00000000u},
    {2, 0x3f800000u, 0x00000000u, 0x3f000000u},
    {3, 0x00000000u, 0x3f800000u, 0x00000000u},
    {4, 0x3f800000u, 0x3f800000u, 0x00000000u},
    {5, 0x3f4ccccdu, 0x3f4ccccdu, 0x3f4ccccdu},
    {6, 0x3f800000u, 0x00000000u, 0x00000000u},
    {7, 0x00000000u, 0x00000000u, 0x3f800000u},
    {8, 0x3e4ccccdu, 0x3e4ccccdu, 0x3f666666u},
    {9, 0x3f800000u, 0x00000000u, 0x3f4ccccdu},
    {11, 0x3f800000u, 0x3e99999au, 0x3e99999au},
};

bool swatch(const Swatch* table, int count, int id, float& r, float& g, float& b) {
    for (int i = 0; i < count; ++i) {
        if (table[i].id == id) {
            r = bits_float(table[i].r);
            g = bits_float(table[i].g);
            b = bits_float(table[i].b);
            return true;
        }
    }
    return false;
}

int color_byte(float value) {
    int byte = static_cast<int>(value * 255.0f);
    if (byte < 0) {
        return 0;
    }
    if (byte > 255) {
        return 255;
    }
    return byte;
}

// ʕ •ᴥ•ʔ✿ Two classes average. A doubled class averages the raid swatch with the pure one. ✿ ʕ •ᴥ•ʔ
bool class_channels(int primary, int secondary, int& r, int& g, int& b) {
    float pr = 0;
    float pg = 0;
    float pb = 0;
    if (!swatch(kRaid, static_cast<int>(sizeof(kRaid) / sizeof(kRaid[0])), primary, pr, pg, pb)) {
        return false;
    }
    float rr = pr;
    float rg = pg;
    float rb = pb;
    if (secondary > 0) {
        float sr = 0;
        float sg = 0;
        float sb = 0;
        const Swatch* table = primary == secondary ? kPure : kRaid;
        int count = primary == secondary ? static_cast<int>(sizeof(kPure) / sizeof(kPure[0])) : static_cast<int>(sizeof(kRaid) / sizeof(kRaid[0]));
        if (swatch(table, count, secondary, sr, sg, sb)) {
            rr = (pr + sr) * 0.5f;
            rg = (pg + sg) * 0.5f;
            rb = (pb + sb) * 0.5f;
        }
    }
    r = color_byte(rr);
    g = color_byte(rg);
    b = color_byte(rb);
    return true;
}

bool pair_from_pack(int pack, int& primary, int& secondary) {
    if (pack <= 0 || pack > 0xffff) {
        return false;
    }
    int low = pack & 0xff;
    int high = (pack >> 8) & 0xff;
    bool lo = class_info(low) != nullptr;
    bool hi = class_info(high) != nullptr;
    if (lo && hi) {
        primary = low;
        secondary = high;
        return true;
    }
    if (lo && high == 0) {
        primary = low;
        secondary = 0;
        return true;
    }
    if (hi && low == 0) {
        primary = high;
        secondary = 0;
        return true;
    }
    return false;
}

bool pair_from_name(const char* text, int& primary, int& secondary) {
    if (!text || !text[0]) {
        return false;
    }
    for (const ClassInfo& info : kClasses) {
        if (std::strcmp(text, info.name) == 0 || std::strcmp(text, info.key) == 0) {
            primary = info.id;
            secondary = 0;
            return true;
        }
    }
    static const int kIds[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
    for (int low : kIds) {
        for (int high : kIds) {
            const char* combo = combo_name(low, high);
            if (combo && std::strcmp(text, combo) == 0) {
                primary = low;
                secondary = high;
                return true;
            }
        }
    }
    return false;
}

void push_channels(void* state, int r, int g, int b) {
    g_push_num(state, r / 255.0);
    g_push_num(state, g / 255.0);
    g_push_num(state, b / 255.0);
    char text[16];
    snprintf(text, sizeof(text), "|cff%02x%02x%02x", r, g, b);
    g_push_str(state, text);
}

int cmc_color(void* state) {
    int primary = 0;
    int secondary = 0;
    bool found = false;
    if (g_is_num && g_is_num(state, 1)) {
        found = pair_from_pack(arg_int(state, 1), primary, secondary);
    } else if (g_to_str && g_is_str && g_is_str(state, 1)) {
        found = pair_from_name(g_to_str(state, 1, nullptr), primary, secondary);
    }
    int r = 0;
    int g = 0;
    int b = 0;
    if (!found || !class_channels(primary, secondary, r, g, b)) {
        g_push_nil(state);
        return 1;
    }
    push_channels(state, r, g, b);
    return 4;
}

void apply_saved_talents();
void refresh_talent_tabs();

int cmc_talents(void* state) {
    int index = arg_int(state, 1);
    int slot = index - 1;
    if (slot < 0 || slot > 9 || slot >= g_class_count || !g_classes[slot].id) {
        g_push_bool(state, 0);
        return 1;
    }
    set_num(22, 2, index);
    refresh_talent_tabs();
    apply_saved_talents();
    syn_net_notify(7, static_cast<uint32_t>(slot), nullptr);
    g_push_bool(state, 1);
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

int g_tradeskill_filter = 0;

int native_tradeskill_filter(void* state) {
    if (g_is_num && g_is_num(state, 1)) {
        g_tradeskill_filter = arg_int(state, 1);
        return 0;
    }
    g_push_num(state, g_tradeskill_filter);
    return 1;
}

double game_num(int cat, int key) {
    GameCat* cat_row = game_cat(cat);
    if (!cat_row) {
        return 0;
    }
    auto found = cat_row->nums.find(key);
    return found == cat_row->nums.end() ? 0 : found->second;
}

struct ItemRow {
    bool on = false;
    uint8_t klass = 0;
    uint8_t quality = 0;
    uint8_t sub = 0;
    uint8_t sound = 0xff;
    uint8_t material = 0xff;
    uint8_t inv = 0;
    uint8_t sheath = 0;
    uint8_t flags = 0;
    uint8_t stat_type = 0xff;
    uint16_t class_mask = 0;
    uint16_t race_mask = 0;
    uint32_t display = 0;
    uint32_t level = 0;
    uint32_t tag_word = 0;
    uint32_t affix_a = 0;
    uint32_t affix_b = 0;
    float stat_value = 0;
    std::string name;
};

std::vector<ItemRow> g_items;

const ItemRow* item_row(int id) {
    id &= 0xffff;
    if (id < 0 || id >= static_cast<int>(g_items.size()) || !g_items[static_cast<size_t>(id)].on) {
        return nullptr;
    }
    return &g_items[static_cast<size_t>(id)];
}

int native_item_tags(void* state) {
    const ItemRow* row = item_row(arg_int(state, 1));
    uint32_t word = row ? row->tag_word : 0;
    for (int i = 0; i < 4; ++i) {
        g_push_num(state, (word >> (8 * i)) & 0xff);
    }
    return 4;
}

int native_item_extra(void* state) {
    const ItemRow* row = item_row(arg_int(state, 1));
    g_push_num(state, row ? row->class_mask : 0);
    g_push_num(state, row ? row->race_mask : 0);
    return 2;
}

int native_id_from_link(void* state) {
    const char* text = g_to_str && g_is_str && g_is_str(state, 1) ? g_to_str(state, 1, nullptr) : nullptr;
    const char* mark = text ? strstr(text, "item:") : nullptr;
    g_push_num(state, mark ? atoi(mark + 5) : 0);
    return 1;
}

int native_highest_attune(void* state) {
    int item = arg_int(state, 1) & 0xffff;
    double best = 0;
    GameCat* cat = game_cat(11);
    if (cat) {
        for (const auto& entry : cat->nums) {
            if ((entry.first & 0xffff) == item && entry.second > best) {
                best = entry.second;
            }
        }
    }
    g_push_num(state, best);
    return 1;
}

int native_in_progress_keys(void* state) {
    (void)state;
    int count = 0;
    GameCat* cat = game_cat(11);
    if (cat) {
        for (const auto& entry : cat->nums) {
            if (entry.second > 0 && entry.second < 100.0) {
                ++count;
            }
        }
    }
    g_push_num(state, count);
    return 1;
}

int native_attunable_count(void* state) {
    (void)state;
    int count = 0;
    for (const ItemRow& row : g_items) {
        if (row.on && (row.tag_word & 0x40)) {
            ++count;
        }
    }
    g_push_num(state, count);
    return 1;
}

int native_attuned_high(void* state) {
    (void)state;
    int count = 0;
    GameCat* cat = game_cat(11);
    if (cat) {
        for (const auto& entry : cat->nums) {
            if (((static_cast<unsigned>(entry.first) >> 24) & 0xff) == 3 && entry.second >= 100.0) {
                ++count;
            }
        }
    }
    g_push_num(state, count);
    return 1;
}

int native_attune_overwrite(void* state) {
    int id = arg_int(state, 1) & 0xffff;
    const ItemRow* row = item_row(id);
    if (!row) {
        g_push_num(state, 0);
        g_push_num(state, 0);
        g_push_num(state, 255);
        g_push_num(state, 0);
        return 4;
    }
    g_push_num(state, id);
    g_push_num(state, row->flags);
    g_push_num(state, row->stat_type);
    g_push_num(state, row->stat_value);
    return 4;
}

void affix_word(int key, uint32_t& low_out, uint32_t& high_out) {
    uint32_t low = static_cast<uint32_t>(game_num(16, key));
    uint32_t mid = static_cast<uint32_t>(game_num(17, key));
    uint32_t high = static_cast<uint32_t>(game_num(20, key));
    uint64_t word = (low & 0x3fffffu) | (static_cast<uint64_t>(mid & 0x3fffffu) << 22) | (static_cast<uint64_t>(high & 0xfffffu) << 44);
    if (word == 0) {
        low = static_cast<uint32_t>(game_num(16, key & 0xffff));
        mid = static_cast<uint32_t>(game_num(17, key & 0xffff));
        high = static_cast<uint32_t>(game_num(20, key & 0xffff));
        word = (low & 0x3fffffu) | (static_cast<uint64_t>(mid & 0x3fffffu) << 22) | (static_cast<uint64_t>(high & 0xfffffu) << 44);
    }
    low_out = static_cast<uint32_t>(word);
    high_out = static_cast<uint32_t>(word >> 32);
}

int native_affix_mask(void* state) {
    int key = arg_int(state, 1);
    uint32_t low = 0;
    uint32_t high = 0;
    affix_word(key, low, high);
    int item = key & 0xffff;
    int forge = (static_cast<unsigned>(key) >> 24) & 0xff;
    if (forge > 3) {
        forge = 0;
    }
    bool done = game_num(11, item | (forge << 24)) >= 100.0;
    uint64_t word = static_cast<uint64_t>(low) | (static_cast<uint64_t>(high) << 32);
    const ItemRow* row = item_row(key);
    g_push_num(state, row ? row->affix_a : low);
    g_push_num(state, row ? row->affix_b : high);
    g_push_num(state, done ? low : 0);
    g_push_num(state, done ? high : 0);
    g_push_num(state, word ? __builtin_ctzll(word) + 1 : 0);
    return 5;
}

const char* db_record_string(const char* symbol, int id, int name_offset) {
    if (id <= 0) {
        return nullptr;
    }
    static std::map<const char*, uint8_t*> tables;
    uint8_t*& table = tables[symbol];
    if (!table) {
        void* found = nullptr;
        if (!syn_resolve(symbol, &found) || !found) {
            return nullptr;
        }
        table = static_cast<uint8_t*>(found);
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    void** by_id = nullptr;
    // ʕ •ᴥ•ʔ✿ 64-bit WowClientDB: max at +0x10, min at +0x14, id index at +0x30. ✿ ʕ •ᴥ•ʔ
    memcpy(&max_id, table + 0x10, 4);
    memcpy(&min_id, table + 0x14, 4);
    memcpy(&by_id, table + 0x30, sizeof(by_id));
    if (!by_id || id < min_id || id > max_id) {
        return nullptr;
    }
    uint8_t* row = static_cast<uint8_t*>(by_id[id - min_id]);
    if (!row) {
        return nullptr;
    }
    const char* name = nullptr;
    memcpy(&name, row + name_offset, sizeof(name));
    return name && name[0] ? name : nullptr;
}

int native_affix_name(void* state) {
    int id = arg_int(state, 1);
    const char* name = nullptr;
    if (id < 0) {
        name = db_record_string("g_itemRandomSuffixDB", -id, 8);
    } else if (id > 0) {
        name = db_record_string("g_itemRandomPropertiesDB", id, 0x28);
    }
    if (!name) {
        g_push_nil(state);
        return 1;
    }
    g_push_str(state, name);
    return 1;
}

int native_attunable_someone(void* state) {
    const ItemRow* row = item_row(arg_int(state, 1));
    g_push_bool(state, row && (row->tag_word & 0x40) ? 1 : 0);
    return 1;
}

// ʕ •ᴥ•ʔ✿ Inv types WoWExt accepts for attune. Shirt, bag, tabard, and ammo are out. ✿ ʕ •ᴥ•ʔ
bool attune_inv_ok(int inv) {
    if (inv > 27) {
        return true;
    }
    return (0x6F3FFEEu >> inv) & 1u;
}

bool attune_weapon_ok(int cls, int inv, int sub, int klass) {
    if (cls < 1 || cls > 11) {
        return false;
    }
    if (inv == 22) {
        const uint32_t offhand = (1u << 0) | (1u << 2) | (1u << 3) | (1u << 5) | (1u << 6);
        if (((offhand >> (cls - 1)) & 1u) == 0) {
            return false;
        }
    } else if (inv == 14 && cls != 1 && cls != 2 && cls != 7) {
        return false;
    }
    auto allow = [&](int limit, uint32_t mask) {
        return cls <= limit && ((mask >> (cls - 1)) & 1u);
    };
    if (klass == 2) {
        bool ok = true;
        switch (sub) {
        case 19:
            ok = cls == 5 || cls == 8 || cls == 9;
            break;
        case 2:
        case 18:
        case 3:
        case 16:
            ok = cls == 1 || cls == 3 || cls == 4;
            break;
        case 6:
            ok = allow(11, 0x427);
            break;
        case 5:
            ok = allow(11, 0x463);
            break;
        case 4:
            ok = allow(11, 0x47b);
            break;
        case 15:
            ok = allow(11, 0x5dd);
            break;
        case 10:
            ok = allow(11, 0x5d5);
            break;
        case 7:
            ok = allow(9, 0x1af);
            break;
        case 1:
            ok = allow(7, 0x67);
            break;
        case 8:
            ok = allow(6, 0x27);
            break;
        case 0:
            ok = allow(7, 0x6f);
            break;
        case 13:
            ok = allow(11, 0x44d);
            break;
        default:
            break;
        }
        if (!ok) {
            return false;
        }
    }
    if (inv == 28) {
        if (sub == 7) {
            return cls == 2;
        }
        if (sub == 8) {
            return cls == 11;
        }
        if (sub == 9) {
            return cls == 7;
        }
        if (sub == 10) {
            return cls == 6;
        }
    }
    return true;
}

bool attune_armor_ok(int cls, int sub, int level) {
    int expect = 1;
    int alt = 0;
    if (cls == 1 || cls == 2 || cls == 6) {
        expect = 4;
        if (level < 45) {
            alt = 3;
        }
    } else if (cls == 11 || cls == 4) {
        expect = 2;
    } else if (cls == 3 || cls == 7) {
        expect = level < 45 ? 2 : 3;
    }
    return expect == sub || (alt && alt == sub);
}

int attune_class_bits(int class_arg, int extra) {
    if (extra & 1) {
        return class_arg;
    }
    if (class_arg < 0) {
        int mask = 0;
        int count = g_class_count < 10 ? g_class_count : 10;
        for (int i = 0; i < count; ++i) {
            int id = g_classes[i].id;
            if (id >= 1 && id <= 11) {
                mask |= 1 << (id - 1);
            }
        }
        return mask;
    }
    if (class_arg > 0) {
        int mask = 0;
        int lo = class_arg & 0xf;
        int hi = (class_arg >> 4) & 0xf;
        if (lo > 0) {
            mask |= 1 << (lo - 1);
        }
        if (hi > 0) {
            mask |= 1 << (hi - 1);
        }
        return mask;
    }
    return 0;
}

int native_can_attune(void* state) {
    bool has_class = g_is_num && g_is_num(state, 2);
    bool has_race = g_is_num && g_is_num(state, 3);
    bool has_extra = g_is_num && g_is_num(state, 5);
    int class_arg = has_class ? arg_int(state, 2) : -1;
    int race_arg = has_race ? arg_int(state, 3) : -1;
    int extra = has_extra ? arg_int(state, 5) : 0;
    bool server = g_is_num && g_is_num(state, 4) && arg_int(state, 4) > 0;
    const ItemRow* row = item_row(arg_int(state, 1));
    int status = server ? -1 : 0;
    if (!row || (row->tag_word & 0x40) == 0 || !attune_inv_ok(row->inv)) {
        g_push_num(state, status);
        return 1;
    }
    if (row->flags & 0x01) {
        g_push_num(state, server ? -1 : -5);
        return 1;
    }
    if (row->flags & 0x08) {
        g_push_num(state, server ? -1 : -3);
        return 1;
    }
    int bits = attune_class_bits(class_arg, extra);
    if (row->inv != 16 && bits != 0) {
        if (row->klass == 4 && row->sub >= 1 && row->sub <= 4) {
            bool matched = false;
            for (int cls = 1; cls <= 11; ++cls) {
                if ((bits & (1 << (cls - 1))) && attune_armor_ok(cls, row->sub, static_cast<int>(row->level))) {
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                g_push_num(state, server ? 0 : -2);
                return 1;
            }
        }
        for (int cls = 1; cls <= 11; ++cls) {
            if ((bits & (1 << (cls - 1))) == 0) {
                continue;
            }
            if (!attune_weapon_ok(cls, row->inv, row->sub, row->klass)) {
                g_push_num(state, server ? 0 : -6);
                return 1;
            }
        }
    }
    if (bits == 0 && class_arg != 0) {
        g_push_num(state, 0);
        return 1;
    }
    if (class_arg == 0) {
        g_push_num(state, 1);
        return 1;
    }
    uint32_t class_mask = row->class_mask & 0x5ffu;
    if (class_mask != 0 && class_mask != 0x5ffu && (class_mask & static_cast<uint32_t>(bits)) == 0) {
        g_push_num(state, server ? 0 : -7);
        return 1;
    }
    if (race_arg == 0) {
        g_push_num(state, 1);
        return 1;
    }
    uint32_t race_mask = row->race_mask & 0x6ffu;
    if (race_mask == 0 || race_mask == 0x6ffu) {
        g_push_num(state, 1);
        return 1;
    }
    bool race_ok = false;
    if (extra & 2) {
        race_ok = (static_cast<uint32_t>(race_arg) & race_mask) != 0;
    } else if (race_arg < 0) {
        race_ok = true;
    } else if (race_arg >= 1 && race_arg <= 31) {
        race_ok = (race_mask & (1u << (race_arg - 1))) != 0;
    }
    g_push_num(state, race_ok ? 1 : (server ? 0 : -8));
    return 1;
}

int native_attuned_affix_count(void* state) {
    (void)state;
    int totals[4] = {};
    for (int cat : {16, 17, 20}) {
        GameCat* row = game_cat(cat);
        if (!row) {
            continue;
        }
        for (const auto& entry : row->nums) {
            int forge = (static_cast<unsigned>(entry.first) >> 24) & 0xff;
            if (forge > 3) {
                forge = 0;
            }
            totals[forge] += __builtin_popcount(static_cast<uint32_t>(entry.second + 0.01));
        }
    }
    for (int total : totals) {
        g_push_num(state, total);
    }
    return 4;
}

int forge_of(int key) {
    int forge = (key >> 24) & 0xff;
    return forge >= 0 && forge <= 3 ? forge : -1;
}

bool attune_hit(int key) {
    if (game_num(11, key) >= 100.0) {
        return true;
    }
    for (int cat : {16, 17, 20}) {
        if (static_cast<int>(game_num(cat, key) + 0.01) != 0) {
            return true;
        }
    }
    return false;
}

int native_has_attuned(void* state) {
    int item = arg_int(state, 1) & 0xffff;
    int filter = -1;
    if (g_is_num && g_is_num(state, 2)) {
        filter = arg_int(state, 2);
    }
    if (item == 0) {
        return 0;
    }
    for (int forge = 0; forge <= 3; ++forge) {
        if (filter >= 0 && forge != filter) {
            continue;
        }
        if (attune_hit(item | (forge << 24))) {
            g_push_num(state, 1);
            return 1;
        }
    }
    return 0;
}

int native_attuned_count(void* state) {
    (void)state;
    int totals[4] = {};
    std::map<int, int> seen;
    GameCat* has = game_cat(11);
    if (has) {
        for (const auto& entry : has->nums) {
            if (entry.second < 100.0) {
                continue;
            }
            int forge = forge_of(entry.first);
            if (forge < 0 || seen[entry.first]) {
                continue;
            }
            seen[entry.first] = 1;
            ++totals[forge];
        }
    }
    for (int cat : {16, 17, 20}) {
        GameCat* row = game_cat(cat);
        if (!row) {
            continue;
        }
        for (const auto& entry : row->nums) {
            if (static_cast<int>(entry.second + 0.01) == 0) {
                continue;
            }
            int forge = forge_of(entry.first);
            if (forge < 0 || seen[entry.first]) {
                continue;
            }
            seen[entry.first] = 1;
            ++totals[forge];
        }
    }
    for (int total : totals) {
        g_push_num(state, total);
    }
    return 4;
}

int native_attunable_affixes(void* state) {
    int count = static_cast<int>(game_num(0x23, 200) + 0.01);
    if (count <= 0) {
        static std::time_t asked = 0;
        std::time_t now = std::time(nullptr);
        if (now - asked > 3) {
            asked = now;
            syn_net_notify(1, 2, nullptr);
        }
        return 0;
    }
    g_push_num(state, count);
    return 1;
}

int native_obj_loaded(void* state) {
    g_push_num(state, g_obj_loaded ? 1 : 0);
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
    g_push_num(state, g_itemloc_loaded ? 1 : 0);
    return 1;
}

int native_zero(void* state) {
    g_push_num(state, 0);
    return 1;
}

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
    if (!g_art) {
        syn_resolve("_ZN8CGItem_C15GetInventoryArtEi", reinterpret_cast<void**>(&g_art));
    }
    const char* art = display > 0 && g_art ? g_art(display) : nullptr;
    if (!art || !art[0]) {
        art = "INV_Misc_QuestionMark";
    }
    using LookupFn = const char* (*)(unsigned);
    static LookupFn lookup = nullptr;
    if (!lookup) {
        syn_resolve("_Z20ClientDBStringLookupj", reinterpret_cast<void**>(&lookup));
    }
    const char* path = lookup ? lookup(3) : nullptr;
    static char texture[260];
    if (path && path[0]) {
        snprintf(texture, sizeof(texture), "%s\\%s", path, art);
    } else {
        snprintf(texture, sizeof(texture), "Interface\\Icons\\%s", art);
    }
    return texture;
}

int native_item_info(void* state) {
    int id = arg_int(state, 1);
    if (id <= 0 && g_is_str && g_is_str(state, 1)) {
        const char* text = g_to_str(state, 1, nullptr);
        const char* mark = text ? strstr(text, "item:") : nullptr;
        if (mark) {
            id = atoi(mark + 5);
        }
    }
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
            row.sound = in.u8();
        }
        if (head & 0x08) {
            row.material = in.u8();
        }
        row.inv = (head & 0x10) ? in.u8() : 0;
        if (head & 0x20) {
            row.sheath = in.u8();
        }
        if (head & 0x40) {
            row.class_mask = in.u16();
        }
        if (head & 0x80) {
            row.race_mask = in.u16();
        }
        row.level = (mask & 0x01) ? in.u8() : in.u16();
        if (mask & 0x02) {
            in.u8();
        }
        if (mask & 0x04) {
            row.flags = in.u8();
        }
        if (mask & 0x08) {
            row.stat_type = in.u8();
            uint32_t bits = in.u32();
            memcpy(&row.stat_value, &bits, 4);
        }
        row.display = (mask & 0x10) ? in.u16() : in.u32();
        row.tag_word = in.u32();
        if (mask & 0x20) {
            row.affix_a = in.u32();
            row.affix_b = in.u32();
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

void load_quest_data(const uint8_t* data, size_t size);
void load_itemloc_obj_at(const uint8_t* data, size_t size);
void load_itemloc_source_at(const uint8_t* data, size_t size);
void load_itemloc_source_count(const uint8_t* data, size_t size);
void load_quest_req(const uint8_t* data, size_t size);
void load_itemloc_item_in_zone(const uint8_t* data, size_t size);
void load_itemloc_item_in_multi_zone(const uint8_t* data, size_t size);
void load_itemloc_source_in_zone(const uint8_t* data, size_t size);
void load_itemloc_fixup_obj_id(const uint8_t* data, size_t size);
void load_itemloc_all_objs_in_zone(const uint8_t* data, size_t size);
void load_itemloc_all_items_in_zone(const uint8_t* data, size_t size);
void load_itemloc_sources_compacted(const uint8_t* data, size_t size);
void load_have_any_reward(const uint8_t* data, size_t size);

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
    load_quest_data(data, size);
    load_itemloc_obj_at(data, size);
    load_itemloc_source_at(data, size);
    load_itemloc_source_count(data, size);
    load_quest_req(data, size);
    load_itemloc_item_in_zone(data, size);
    load_itemloc_item_in_multi_zone(data, size);
    load_itemloc_source_in_zone(data, size);
    load_itemloc_fixup_obj_id(data, size);
    load_itemloc_all_objs_in_zone(data, size);
    load_itemloc_all_items_in_zone(data, size);
    load_itemloc_sources_compacted(data, size);
    load_have_any_reward(data, size);
}

#include "natives/custom_get_current_zone.inc"
#include "natives/custom_get_zone_name.inc"
#include "natives/custom_zone_contains.inc"
#include "natives/custom_get_quest_name.inc"
#include "natives/custom_is_on_quest.inc"
#include "natives/custom_get_quest_data.inc"
#include "natives/itemloc_obj_at.inc"
#include "natives/itemloc_obj_count.inc"
#include "natives/itemloc_set_obj_sort.inc"
#include "natives/itemloc_source_at.inc"
#include "natives/itemloc_get_source_sort.inc"
#include "natives/itemloc_set_source_sort.inc"
#include "natives/itemloc_fixup_obj_id.inc"
#include "natives/custom_get_quest_faction.inc"
#include "natives/custom_have_any_reward.inc"
#include "natives/custom_get_current_zone_our.inc"
#include "natives/custom_get_quest_completed.inc"
#include "natives/itemloc_source_count.inc"
#include "natives/custom_get_spell_desc.inc"
#include "natives/custom_get_spell_cooldown.inc"
#include "natives/custom_get_highest_spell.inc"
#include "natives/custom_get_spell_effect.inc"
#include "natives/custom_get_spell_tools.inc"
#include "natives/custom_get_item_count.inc"
#include "natives/custom_is_item_equip_mgr.inc"
#include "natives/custom_is_have_item.inc"
#include "natives/custom_cache_have_items.inc"
#include "natives/custom_get_have_items.inc"
#include "natives/custom_is_item_soulbound.inc"
#include "natives/custom_get_merchant_item.inc"
#include "natives/custom_get_quest_req.inc"
#include "natives/custom_get_prequest.inc"
#include "natives/custom_get_quest_can_accept.inc"
#include "natives/itemloc_all_objs_in_zone.inc"
#include "natives/itemloc_all_items_in_zone.inc"
#include "natives/itemloc_sources_compacted.inc"
#include "natives/custom_get_cursor_item.inc"
#include "natives/custom_get_item_guid.inc"
#include "natives/custom_get_item_link_by_slot.inc"
#include "natives/custom_send_quest_track.inc"
#include "natives/custom_add_track_obj_loc.inc"
#include "natives/custom_remove_track_obj_loc.inc"
#include "natives/custom_clear_track_obj_loc.inc"
#include "natives/custom_get_all_track_obj_loc.inc"
#include "natives/custom_calculate_map_marker.inc"
#include "natives/custom_get_total_map_marker_count.inc"
#include "natives/custom_get_map_marker.inc"
#include "natives/custom_set_map_marker_option.inc"
#include "natives/custom_is_player_near.inc"
#include "natives/custom_is_container_soulbound.inc"
#include "natives/custom_get_pending_events.inc"
#include "natives/custom_get_quest_item_id.inc"
#include "natives/custom_get_tradeskill_tools.inc"
#include "natives/custom_queue_event.inc"
#include "natives/custom_get_pending_events_ex.inc"
#include "natives/custom_queue_event_ex.inc"
#include "natives/custom_get_entity_extra_int.inc"
#include "natives/custom_get_entity_extra_float.inc"
#include "natives/custom_set_entity_extra_int.inc"
#include "natives/custom_set_entity_extra_float.inc"
#include "natives/custom_set_result_use_container_item.inc"
#include "natives/custom_get_loot_filter.inc"
#include "natives/custom_get_item_vars.inc"
#include "natives/custom_has_acquired_item.inc"
#include "natives/custom_get_item_id_from_name.inc"
#include "natives/custom_unregister_use_container_item.inc"
#include "natives/custom_register_use_container_item.inc"
#include "natives/custom_get_loot_filter_rule.inc"
#include "natives/custom_set_loot_filter.inc"
#include "natives/custom_get_loot_filter_rule_entry.inc"
#include "natives/custom_loot_filter_reset.inc"
#include "natives/custom_get_skill_index.inc"
#include "natives/custom_get_gossip_quests.inc"
#include "natives/itemloc_item_in_zone.inc"
#include "natives/itemloc_item_in_multi_zone.inc"
#include "natives/itemloc_source_in_zone.inc"
#include "natives/custom_translate_map_position.inc"
#include "natives/custom_get_map_filtered.inc"
#include "natives/custom_get_profession_recipe_info.inc"
#include "natives/custom_get_profession_recipe_reagents.inc"
#include "natives/custom_get_profession_recipe_from_crafted.inc"
#include "natives/custom_do_profession_recipe.inc"
#include "natives/custom_get_profession_recipes.inc"
#include "natives/custom_set_loot_filter_rule_enabled.inc"
#include "natives/custom_set_loot_filter_rule_entry.inc"
#include "natives/custom_set_loot_filter_options.inc"
#include "natives/custom_set_loot_filter_rule.inc"
#include "natives/custom_loot_filter_export.inc"
#include "natives/custom_loot_filter_import.inc"
#include "natives/custom_loot_filter_import_rule.inc"
#include "natives/custom_get_item_select_count.inc"
#include "natives/custom_set_item_select_filter.inc"
#include "natives/custom_get_item_select.inc"
#include "natives/custom_clear_dressup_facing.inc"
#include "natives/custom_set_dressup_facing.inc"
#include "natives/debug_set_enable_zone_item_cache.inc"
#include "natives/get_safe_invoke_queue.inc"
#include "natives/set_item_tracker_class_mask.inc"
#include "natives/custom_get_transmog_enchant.inc"
#include "natives/custom_get_transmog_item.inc"
#include "natives/custom_fixup_transmog_id.inc"
#include "natives/custom_transmog_export.inc"
#include "natives/custom_loot_filter_export_rule.inc"
#include "natives/custom_lock_item.inc"
#include "natives/custom_unlock_item.inc"

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
    register_cmc("GetAffixNameCustom", native_affix_name);
    register_cmc("Custom_GetZoneName", native_get_zone_name);
    register_cmc("Custom_ZoneContains", native_zone_contains);
    register_cmc("Custom_GetCurrentZone", native_get_current_zone);
    register_cmc("Custom_GetCurrentZoneOur", native_get_current_zone_our);
    register_cmc("Custom_GetQuestName", native_get_quest_name);
    register_cmc("Custom_IsOnQuest", native_is_on_quest);
    register_cmc("Custom_GetQuestData", native_get_quest_data);
    register_cmc("ItemLocGetObjAt", native_itemloc_obj_at);
    register_cmc("ItemLocGetObjCount", native_itemloc_obj_count);
    register_cmc("ItemLocSetObjSort", native_itemloc_set_obj_sort);
    register_cmc("ItemLocGetSourceAt", native_itemloc_source_at);
    register_cmc("ItemLocGetSourceSort", native_itemloc_get_source_sort);
    register_cmc("ItemLocSetSourceSort", native_itemloc_set_source_sort);
    register_cmc("ItemLocFixupObjId", native_itemloc_fixup_obj_id);
    register_cmc("Custom_GetQuestFaction", native_get_quest_faction);
    register_cmc("Custom_HaveAnyRewardOfQuest", native_have_any_reward);
    register_cmc("Custom_GetQuestCompleted", native_get_quest_completed);
    register_cmc("ItemLocGetSourceCount", native_itemloc_source_count);
    register_cmc("Custom_GetItemCount", native_get_item_count);
    register_cmc("Custom_IsItemEquipMgr", native_is_item_equip_mgr);
    register_cmc("Custom_IsHaveItem", native_is_have_item);
    register_cmc("Custom_CacheHaveItems", native_cache_have_items);
    register_cmc("Custom_GetHaveItems", native_get_have_items);
    register_cmc("Custom_IsItemSoulbound", native_is_item_soulbound);
    register_cmc("Custom_GetQuestReq", native_get_quest_req);
    register_cmc("Custom_GetPrequest", native_get_prequest);
    register_cmc("Custom_GetQuestCanAccept", native_get_quest_can_accept);
    register_cmc("ItemLocItemIsInZone", native_itemloc_item_in_zone);
    register_cmc("ItemLocItemIsInMultiZone", native_itemloc_item_in_multi_zone);
    register_cmc("ItemLocSourceIsInZone", native_itemloc_source_in_zone);
    register_cmc("ItemLocGetAllObjsInZone", native_itemloc_all_objs_in_zone);
    register_cmc("ItemLocGetAllItemsInZone", native_itemloc_all_items_in_zone);
    register_cmc("ItemLocGetSourcesCompacted", native_itemloc_sources_compacted);
    register_cmc("Custom_GetCurrentCursorItem", native_get_cursor_item);
    register_cmc("Custom_GetItemGuid", native_get_item_guid);
    register_cmc("Custom_GetItemLinkBySlot", native_get_item_link_by_slot);
    register_cmc("Custom_SendQuestTrack", native_send_quest_track);
    register_cmc("Custom_AddTrackObjLoc", native_add_track_obj_loc);
    register_cmc("Custom_RemoveTrackObjLoc", native_remove_track_obj_loc);
    register_cmc("Custom_ClearTrackObjLoc", native_clear_track_obj_loc);
    register_cmc("Custom_CalculateMapMarker", native_calculate_map_marker);
    register_cmc("Custom_IsPlayerNear", native_is_player_near);
    register_cmc("Custom_IsContainerItemSoulbound", native_is_container_soulbound);
    register_cmc("Custom_GetPendingEvents", native_get_pending_events);
    register_cmc("Custom_GetPendingEventsEx", native_get_pending_events_ex);
    register_cmc("Custom_GetEntityExtraDataInt", native_get_entity_extra_int);
    register_cmc("Custom_GetEntityExtraDataFloat", native_get_entity_extra_float);
    register_cmc("Custom_SetEntityExtraDataInt", native_set_entity_extra_int);
    register_cmc("Custom_SetEntityExtraDataFloat", native_set_entity_extra_float);
    register_cmc("Custom_Internal_SetResultFromUseContainerItem", native_set_result_use_container_item);
    register_cmc("Custom_GetLootFilter", native_get_loot_filter);
    register_cmc("Custom_GetLootFilterRule", native_get_loot_filter_rule);
    register_cmc("Custom_SetLootFilter", native_set_loot_filter);
    register_cmc("Custom_GetLootFilterRuleEntry", native_get_loot_filter_rule_entry);
    register_cmc("Custom_LootFilterResetToDefault", native_loot_filter_reset);
    register_cmc("Custom_GetItemVars", native_get_item_vars);
    register_cmc("Custom_HasAcquiredItem", native_has_acquired_item);
    register_cmc("Custom_UnregisterForUseContainerItem", native_unregister_use_container_item);
    register_cmc("Custom_RegisterForUseContainerItem", native_register_use_container_item);
    register_cmc("Custom_GetQuestItemId", native_get_quest_item_id);
    register_cmc("Custom_GetTradeSkillTools", native_get_tradeskill_tools);
    register_cmc("Custom_QueueEvent", native_queue_event);
    register_cmc("Custom_QueueEventEx", native_queue_event_ex);
    register_cmc("Custom_GetSkillIndex", native_get_skill_index);
    register_cmc("Custom_GetGossipQuests", native_get_gossip_quests);
    register_cmc("Custom_GetMerchantItem", native_get_merchant_item);
    register_cmc("Custom_GetSpellDesc", native_get_spell_desc);
    register_cmc("Custom_GetSpellCooldown", native_get_spell_cooldown);
    register_cmc("Custom_GetHighestLearnedSpell", native_get_highest_spell);
    register_cmc("Custom_GetSpellEffect", native_get_spell_effect);
    register_cmc("Custom_GetSpellTools", native_get_spell_tools);
    register_cmc("Custom_TranslateMapPosition", native_translate_map_position);
    register_cmc("Custom_GetMapFilteredResults", native_get_map_filtered);
    register_cmc("Custom_GetProfessionRecipeInfo", native_get_profession_recipe_info);
    register_cmc("Custom_GetProfessionRecipeReagents", native_get_profession_recipe_reagents);
    register_cmc("Custom_GetProfessionRecipeFromCraftedItem", native_get_profession_recipe_from_crafted);
    register_cmc("Custom_DoProfessionRecipe", native_do_profession_recipe);
    register_cmc("Custom_GetProfessionRecipes", native_get_profession_recipes);
    register_cmc("Custom_SetLootFilterRuleEnabled", native_set_loot_filter_rule_enabled);
    register_cmc("Custom_SetLootFilterRuleEntry", native_set_loot_filter_rule_entry);
    register_cmc("Custom_SetLootFilterOptions", native_set_loot_filter_options);
    register_cmc("Custom_SetLootFilterRule", native_set_loot_filter_rule);
    register_cmc("Custom_LootFilterExport", native_loot_filter_export);
    register_cmc("Custom_LootFilterImport", native_loot_filter_import);
    register_cmc("Custom_LootFilterImportRule", native_loot_filter_import_rule);
    register_cmc("Custom_LootFilterExportRule", native_loot_filter_export_rule);
    register_cmc("Custom_GetItemSelectCount", native_get_item_select_count);
    register_cmc("Custom_SetItemSelectFilter", native_set_item_select_filter);
    register_cmc("Custom_GetItemSelect", native_get_item_select);
    register_cmc("Custom_ClearDressupFacing", native_clear_dressup_facing);
    register_cmc("Custom_SetDressupFacing", native_set_dressup_facing);
    register_cmc("Custom_LockItem", native_lock_item);
    register_cmc("Custom_UnlockItem", native_unlock_item);
    register_cmc("GetAttuneOverwrite", native_attune_overwrite);
    register_cmc("GetHighestAttunePct", native_highest_attune);
    register_cmc("GetInProgressAttuneKeys", native_in_progress_keys);
    register_cmc("GetSafeInvokeQueue", native_safe_invoke_queue);
    register_cmc("CalculateAttunableCount", native_attunable_count);
    register_cmc("CalculateAttunableAffixCount", native_attunable_affixes);
    register_cmc("CalculateAttunedCount", native_attuned_count);
    register_cmc("CalculateAttunedAffixCount", native_attuned_affix_count);
    register_cmc("CalculateAttunedCountHigh", native_attuned_high);
    register_cmc("Debug_SetEnableZoneItemCache", native_debug_zone_item_cache);
    register_cmc("Custom_GetIdFromLink", native_id_from_link);
    register_cmc("CanAttuneItemHelperEx", native_can_attune);
    register_cmc("HasAttunedAnyVariantEx", native_has_attuned);
    register_cmc("IsAttunableBySomeone", native_attunable_someone);
    register_cmc("GetItemTagsCustom", native_item_tags);
    register_cmc("GetItemExtraCustom", native_item_extra);
    register_cmc("GetItemAffixMask", native_affix_mask);
    register_cmc("SetItemTrackerClassMask", native_item_tracker_class_mask);
    register_cmc("Custom_GetTransmogEnchant", native_get_transmog_enchant);
    register_cmc("Custom_GetTransmogItem", native_get_transmog_item);
    register_cmc("Custom_FixupTransmogId", native_fixup_transmog_id);
    register_cmc("Custom_TransmogExport", native_transmog_export);
    register_cmc("Custom_SetTradeSkillCustomFilter", native_tradeskill_filter);
    register_cmc("Custom_GetAllTrackObjLoc", native_get_all_track_obj_loc);
    register_cmc("Custom_GetMapMarker", native_get_map_marker);
    register_cmc("Custom_GetTotalMapMarkerCount", native_get_total_map_marker_count);
    register_cmc("Custom_SetMapMarkerOption", native_set_map_marker_option);
    zone_cache_off();
    g_natives_ready = true;
    log_line("registered CMC natives");
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
    if (!g_perk_lua.empty()) {
        std::string wrapped = "SynastriaScriptError = nil\nlocal ok, err = pcall(function()\nPerkMgrPerks = PerkMgrPerks or {}\nPerkMgrSets = PerkMgrSets or {}\n";
        wrapped += g_perk_lua;
        wrapped += "\nend)\nif not ok then SynastriaScriptError = tostring(err) end\n";
        g_exec(wrapped.c_str(), "WoWExt.bin", "perks");
        if (g_context && g_get_field && g_to_str && g_set_top) {
            void* state = g_context();
            if (state) {
                g_get_field(state, kLuaGlobals, "SynastriaScriptError");
                const char* err = g_to_str(state, -1, nullptr);
                if (err && err[0]) {
                    log_line("perk lua error: %s", err);
                }
                g_set_top(state, -2);
            }
        }
    }
    if (g_scripts.empty()) {
        log_line("container scripts were empty");
        g_scripts_ready = true;
        return;
    }
    int errors = 0;
    for (const LuaScript& script : g_scripts) {
        std::string source = script.source;
        const std::string class_id = "local function GetClassId()\r\n\tlocal name, keyword = UnitClass(\"player\")\r\n";
        const std::string class_from_list =
            "local function GetClassId()\r\n"
            "\tlocal ok, id = pcall(CMCGetClassAt, 1)\r\n"
            "\tif ok and type(id) == \"number\" and id > 0 then return id end\r\n"
            "\tlocal name, keyword = UnitClass(\"player\")\r\n";
        size_t at = source.find(class_id);
        if (at != std::string::npos) {
            source.replace(at, class_id.size(), class_from_list);
        }
        const std::string class_rows =
            "\toffset_y, counter = UpdateLeftCat(frame.cele.catcla, offset_y, counter, GetClassId(), 5)\r\n"
            "\tif frame.cele.catclb then\r\n"
            "\t\tlocal scl = CMCGetClassAt(2)\r\n"
            "\t\tif scl and scl ~= 0 and scl ~= GetClassId() then\r\n"
            "\t\t\toffset_y, counter = UpdateLeftCat(frame.cele.catclb, offset_y, counter, scl, 5)\r\n"
            "\t\tend\r\n"
            "\tend\r\n"
            "\toffset_y, counter = UpdateLeftCat(frame.cele.catmis, offset_y, counter, 19, 6)\r\n";
        const std::string class_rows_both =
            "\tlocal classA = CMCGetClassAt(1) or GetClassId()\r\n"
            "\tlocal classB = CMCGetClassAt(2)\r\n"
            "\tif frame.cele.catcla then frame.cele.catcla.isCollapsed = nil end\r\n"
            "\toffset_y, counter = UpdateLeftCat(frame.cele.catcla, offset_y, counter, classA, 5)\r\n"
            "\tif frame.cele.catclb and classB and classB ~= 0 and classB ~= classA then\r\n"
            "\t\tframe.cele.catclb.isCollapsed = nil\r\n"
            "\t\toffset_y, counter = UpdateLeftCat(frame.cele.catclb, offset_y, counter, classB, 5)\r\n"
            "\tend\r\n"
            "\tif frame.cele.catmis then frame.cele.catmis.isCollapsed = nil end\r\n"
            "\toffset_y, counter = UpdateLeftCat(frame.cele.catmis, offset_y, counter, 19, 6)\r\n";
        at = source.find(class_rows);
        if (at != std::string::npos) {
            source.replace(at, class_rows.size(), class_rows_both);
        }
        const std::string cat_title = "\tcat:SetText(c_catname[catidx] .. string.format(\" (%d/%d)\", acount, limit))\r\n";
        const std::string cat_title_named =
            "\tlocal title = c_catname[catidx]\r\n"
            "\tif catidx == 5 and type(CMCGetClassName) == \"function\" then\r\n"
            "\t\tlocal nm = CMCGetClassName(catid)\r\n"
            "\t\tif nm and nm ~= \"\" then title = nm end\r\n"
            "\tend\r\n"
            "\tcat:SetText(title .. string.format(\" (%d/%d)\", acount, limit))\r\n";
        at = source.find(cat_title);
        if (at != std::string::npos) {
            source.replace(at, cat_title.size(), cat_title_named);
        }
        const std::string limit_block =
            "\t\tif catidx ~= 6 and (not GetPerkLimit(catidx) or GetActiveCount(perk.cat) >= GetPerkLimit(catidx)) then\r\n";
        const std::string limit_open =
            "\t\tlocal perkLimit = GetPerkLimit(catidx)\r\n"
            "\t\tif catidx ~= 6 and perkLimit and perkLimit > 0 and GetActiveCount(perk.cat) >= perkLimit then\r\n";
        at = 0;
        while ((at = source.find(limit_block, at)) != std::string::npos) {
            source.replace(at, limit_block.size(), limit_open);
            at += limit_open.size();
        }
        std::string wrapped = "SynastriaScriptError = nil\nlocal ok, err = pcall(function()\n";
        wrapped += source;
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
    g_exec(
        "if not SynastriaViewHook then\n"
        "SynastriaViewHook = true\n"
        "local function open_view(name)\n"
        "  if name ~= \"View Attuned Items\" then return end\n"
        "  SynastriaLastSpell = name\n"
        "  if not OpenAttuneSummary then\n"
        "    SynastriaSpellOpenError = \"OpenAttuneSummary missing\"\n"
        "    return\n"
        "  end\n"
        "  local ok, err = pcall(OpenAttuneSummary)\n"
        "  if not ok then SynastriaSpellOpenError = tostring(err) end\n"
        "end\n"
        "hooksecurefunc(\"CastSpell\", function(index, bookType)\n"
        "  open_view(GetSpellName(index, bookType))\n"
        "end)\n"
        "hooksecurefunc(\"UseAction\", function(slot)\n"
        "  local kind, id = GetActionInfo(slot)\n"
        "  if kind ~= \"spell\" or not id then return end\n"
        "  open_view(GetSpellName(id, BOOKTYPE_SPELL) or (GetSpellInfo and GetSpellInfo(id)))\n"
        "end)\n"
        "end\n",
        "WoWExt.bin", "view");
    if (!g_synergy_lua.empty()) {
        std::string wrapped = "SynastriaScriptError = nil\nlocal ok, err = pcall(function()\nSynergyPerks = SynergyPerks or {}\n";
        wrapped += g_synergy_lua;
        wrapped += "\nend)\nif not ok then SynastriaScriptError = tostring(err) end\n";
        g_exec(wrapped.c_str(), "WoWExt.bin", "synergy");
        if (g_context && g_get_field && g_to_str && g_set_top) {
            void* state = g_context();
            if (state) {
                g_get_field(state, kLuaGlobals, "SynastriaScriptError");
                const char* err = g_to_str(state, -1, nullptr);
                if (err && err[0]) {
                    log_line("synergy lua error: %s", err);
                }
                g_set_top(state, -2);
            }
        }
    }
    g_scripts_ready = true;
    log_line("executed %zu WoWExt.bin lua containers errors=%d", g_scripts.size(), errors);
}

void poll_spell_click() {
    if (!g_context || !g_get_field || !g_to_str || !g_set_top || !g_exec) {
        return;
    }
    void* state = g_context();
    if (!state) {
        return;
    }
    g_get_field(state, kLuaGlobals, "SynastriaLastSpell");
    const char* name = g_to_str(state, -1, nullptr);
    static std::string seen;
    if (name && name[0] && seen != name) {
        seen = name;
        log_line("spellbook click %s", seen.c_str());
    }
    g_set_top(state, -2);
    g_get_field(state, kLuaGlobals, "SynastriaSpellOpenError");
    const char* err = g_to_str(state, -1, nullptr);
    if (err && err[0]) {
        std::string copy = err;
        g_set_top(state, -2);
        g_exec("SynastriaSpellOpenError = nil", "WoWExt.bin", "view");
        log_line("spell open error: %s", copy.c_str());
        return;
    }
    g_set_top(state, -2);
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

struct TalentApply {
    void (*leave)() = nullptr;
    void (*enter)() = nullptr;
    void (*update)(void*) = nullptr;
    void (*ctor)(void*) = nullptr;
    void (*dtor)(void*) = nullptr;
    void (*put)(void*, uint8_t) = nullptr;
    void (*finalize)(void*) = nullptr;
    bool ready = false;
};

TalentApply& talent_apply() {
    static TalentApply api;
    if (api.ready) {
        return api;
    }
    api.ready = true;
    void* found = nullptr;
    if (syn_resolve("_ZN12CGTalentInfo10LeaveWorldEv", &found)) {
        api.leave = reinterpret_cast<void (*)()>(found);
    }
    if (syn_resolve("_ZN12CGTalentInfo10EnterWorldEv", &found)) {
        api.enter = reinterpret_cast<void (*)()>(found);
    }
    if (syn_resolve("_ZN12CGTalentInfo16UpdateTalentDataEP10CDataStore", &found)) {
        api.update = reinterpret_cast<void (*)(void*)>(found);
    }
    if (syn_resolve("_ZN10CDataStoreC1Ev", &found)) {
        api.ctor = reinterpret_cast<void (*)(void*)>(found);
    }
    if (syn_resolve("_ZN10CDataStoreD1Ev", &found)) {
        api.dtor = reinterpret_cast<void (*)(void*)>(found);
    }
    if (syn_resolve("_ZN10CDataStore3PutEh", &found)) {
        api.put = reinterpret_cast<void (*)(void*, uint8_t)>(found);
    }
    if (syn_resolve("_ZN10CDataStore8FinalizeEv", &found)) {
        api.finalize = reinterpret_cast<void (*)(void*)>(found);
    }
    return api;
}

void* active_player() {
    static void* (*get)() = nullptr;
    static bool ready = false;
    if (!ready) {
        ready = true;
        void* found = nullptr;
        if (syn_resolve("_Z28ClntObjMgrGetActivePlayerObjv", &found)) {
            get = reinterpret_cast<void* (*)()>(found);
        }
    }
    return get ? get() : nullptr;
}

void refresh_talent_tabs() {
    TalentApply& tabs = talent_apply();
    if (!g_in_world || !tabs.leave || !tabs.enter) {
        return;
    }
    int slot = static_cast<int>(game_num(22, 2) + 0.01) - 1;
    if (slot < 0) {
        slot = 0;
    }
    uint8_t want = slot < g_class_count ? g_classes[slot].id : 0;
    uint8_t* class_byte = nullptr;
    uint8_t saved = 0;
    void* player = active_player();
    if (player && want) {
        // ʕ •ᴥ•ʔ✿ CGPlayer_C unit data is +0x160, class is unitFieldBytes0[1]. ✿ ʕ •ᴥ•ʔ
        auto* unit = *reinterpret_cast<uint8_t**>(static_cast<char*>(player) + 0x160);
        if (unit) {
            class_byte = unit + 0x45;
            saved = *class_byte;
            *class_byte = want;
        }
    }
    tabs.leave();
    tabs.enter();
    if (class_byte) {
        *class_byte = saved;
    }
    log_line("talent tabs class=%u", want);
}

void apply_saved_talents() {
    int slot = static_cast<int>(game_num(22, 2) + 0.01) - 1;
    if (slot < 0) {
        slot = 0;
    }
    if (slot >= g_class_count || g_class_talents[slot].empty()) {
        return;
    }
    TalentApply& api = talent_apply();
    if (!api.update || !api.ctor || !api.dtor || !api.put || !api.finalize) {
        return;
    }
    alignas(16) unsigned char raw[64];
    memset(raw, 0, sizeof(raw));
    api.ctor(raw);
    for (uint8_t byte : g_class_talents[slot]) {
        api.put(raw, byte);
    }
    api.finalize(raw);
    api.update(raw);
    api.dtor(raw);
    log_line("applied class talent slot=%d bytes=%zu", slot, g_class_talents[slot].size());
}

// ʕ •ᴥ•ʔ✿ Command 14 type 2 is one talent block per class slot. ✿ ʕ •ᴥ•ʔ
void store_class_talents(const uint8_t* rest, uint32_t size) {
    if (size < 2) {
        return;
    }
    uint8_t count = rest[1];
    uint32_t off = 2;
    for (uint8_t i = 0; i < count; ++i) {
        if (off + 5 > size) {
            return;
        }
        uint8_t index = rest[off];
        uint32_t nbytes = 0;
        memcpy(&nbytes, rest + off + 1, 4);
        off += 5;
        if (nbytes > 0x10000u || off + nbytes > size) {
            return;
        }
        if (index < kMaxClasses) {
            g_class_talents[index].assign(rest + off, rest + off + nbytes);
            log_line("class talent slot=%u bytes=%u", index, nbytes);
        }
        off += nbytes;
    }
    apply_saved_talents();
}

void adopt_classes(const uint8_t* rest, uint32_t size) {
    if (!rest || size < 2) {
        return;
    }
    if (rest[0] == 2) {
        store_class_talents(rest, size);
        return;
    }
    if (rest[0] != 1) {
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
    for (std::vector<uint8_t>& blob : g_class_talents) {
        blob.clear();
    }
    log_line("dual class list count=%u first=%u second=%u", count, next[0].id, count > 1 ? next[1].id : 0);
#if defined(SYNASTRIA_EMBED)
    if (g_in_world) {
        CGSpellBook::RefileSpells();
    }
#endif
}

constexpr uint32_t kSpellBytes = 0x2b8;
constexpr int kSpellNameAt = 0x220;
constexpr int kSpellDescAt = 0x230;
constexpr int kSpellAuraAt = 0x238;
constexpr int kSpellIconAt = 0x214;
constexpr int kSpellIdLimit = 0x1fbd0;

struct SpellTable {
    uint8_t* base = nullptr;
    int32_t min_id = 0;
    int32_t max_id = 0;
    uint8_t** by_id = nullptr;
    bool compress = false;
    uint8_t* (*decompress)(const uint8_t*, uint32_t, uint8_t*) = nullptr;
    uint32_t (*compress_row)(const uint8_t*, uint32_t, uint8_t*) = nullptr;
    std::map<int, std::vector<uint8_t>> plain;
    std::map<int, std::vector<uint8_t>> packed;
    std::vector<char*> text;
};

SpellTable& spell_table() {
    static SpellTable table;
    static bool ready = false;
    if (ready) {
        return table;
    }
    void* found = nullptr;
    if (!syn_resolve("g_spellDB", &found) || !found) {
        return table;
    }
    ready = true;
    table.base = static_cast<uint8_t*>(found);
    memcpy(&table.max_id, table.base + 0x10, 4);
    memcpy(&table.min_id, table.base + 0x14, 4);
    memcpy(&table.by_id, table.base + 0x30, sizeof(table.by_id));
    found = nullptr;
    if (syn_resolve("g_clientDbPerformCompress", &found) && found) {
        int32_t flag = 0;
        memcpy(&flag, found, 4);
        table.compress = flag != 0;
    }
    found = nullptr;
    if (syn_resolve("_Z13DecompressRowPKhjPh", &found)) {
        table.decompress = reinterpret_cast<uint8_t* (*)(const uint8_t*, uint32_t, uint8_t*)>(found);
    }
    found = nullptr;
    if (syn_resolve("_Z11CompressRowPKhjPh", &found)) {
        table.compress_row = reinterpret_cast<uint32_t (*)(const uint8_t*, uint32_t, uint8_t*)>(found);
    }
    return table;
}

void spell_grow(SpellTable& table, int id) {
    if (!table.base || id <= table.max_id || id < table.min_id) {
        return;
    }
    size_t grown_n = static_cast<size_t>(id - table.min_id + 1);
    size_t old_n = table.max_id >= table.min_id ? static_cast<size_t>(table.max_id - table.min_id + 1) : 0;
    uint8_t** grown = new uint8_t*[grown_n]();
    if (table.by_id && old_n) {
        memcpy(grown, table.by_id, old_n * sizeof(uint8_t*));
    }
    table.by_id = grown;
    memcpy(table.base + 0x30, &table.by_id, sizeof(table.by_id));
    memcpy(table.base + 0x10, &id, 4);
    table.max_id = id;
}

uint8_t* spell_plain(SpellTable& table, int id) {
    if (!table.by_id || id < table.min_id) {
        return nullptr;
    }
    if (id > table.max_id) {
        spell_grow(table, id);
    }
    if (id > table.max_id) {
        return nullptr;
    }
    auto have = table.plain.find(id);
    if (have != table.plain.end()) {
        return have->second.data();
    }
    uint8_t* raw = table.by_id[id - table.min_id];
    std::vector<uint8_t> row(kSpellBytes);
    if (!raw) {
        int32_t self = id;
        int32_t icon = 1;
        memcpy(row.data(), &self, 4);
        memcpy(row.data() + 0x214, &icon, 4);
        auto stored = table.plain.emplace(id, std::move(row));
        return stored.first->second.data();
    }
    if (table.compress) {
        if (!table.decompress) {
            return nullptr;
        }
        table.decompress(raw, kSpellBytes, row.data());
    } else {
        memcpy(row.data(), raw, kSpellBytes);
    }
    auto stored = table.plain.emplace(id, std::move(row));
    return stored.first->second.data();
}

void spell_fill_strings(uint8_t* row) {
    // ʕ •ᴥ•ʔ✿ The spellbook reads the rank text with no null check. ✿ ʕ •ᴥ•ʔ
    static char empty[] = "";
    char* blank = empty;
    for (int at = kSpellNameAt; at <= kSpellAuraAt; at += static_cast<int>(sizeof(char*))) {
        char* text = nullptr;
        memcpy(&text, row + at, sizeof(text));
        if (!text) {
            memcpy(row + at, &blank, sizeof(blank));
        }
    }
}

void spell_store(SpellTable& table, int id) {
    uint8_t* row = spell_plain(table, id);
    if (!row || !table.by_id) {
        return;
    }
    spell_fill_strings(row);
    // ʕ •ᴥ•ʔ✿ Bit 0x80 hides the spell and, with 0x100, makes GetSpellLink return nothing. ✿ ʕ •ᴥ•ʔ
    int32_t attributes = 0;
    memcpy(&attributes, row + 0x10, 4);
    attributes &= ~0x180;
    memcpy(row + 0x10, &attributes, 4);
    if (!table.compress) {
        table.by_id[id - table.min_id] = row;
        return;
    }
    if (!table.compress_row) {
        return;
    }
    uint32_t bytes = table.compress_row(row, kSpellBytes, nullptr);
    if (bytes == 0 || bytes > 0x2000) {
        return;
    }
    auto& blob = table.packed[id];
    blob.resize(bytes);
    table.compress_row(row, kSpellBytes, blob.data());
    table.by_id[id - table.min_id] = blob.data();
}

void spell_set_text(SpellTable& table, int id, int at, const uint8_t* text, int length) {
    uint8_t* row = spell_plain(table, id);
    if (!row || length <= 0) {
        return;
    }
    char* copy = new char[static_cast<size_t>(length) + 1];
    memcpy(copy, text, static_cast<size_t>(length));
    copy[length] = 0;
    table.text.push_back(copy);
    memcpy(row + at, &copy, sizeof(copy));
    spell_store(table, id);
}

struct SkillAdd {
    uint16_t skill = 0;
    uint32_t spell = 0;
    uint16_t mask = 0;
};

std::vector<SkillAdd> g_skill_adds;
int spell_field_at(uint32_t offset) {
    // ʕ •ᴥ•ʔ✿ Four string pointers grow by 4 bytes on the 64-bit spell row. ✿ ʕ •ᴥ•ʔ
    if (offset >= 0x230) {
        offset += 0x10;
    }
    return static_cast<int>(offset);
}

bool read_varint(const uint8_t* data, int size, int& cursor, uint32_t& out) {
    out = 0;
    int shift = 0;
    while (cursor < size && shift < 28) {
        uint8_t bite = data[cursor++];
        out |= static_cast<uint32_t>(bite & 0x7f) << shift;
        if ((bite & 0x80) == 0) {
            return true;
        }
        shift += 7;
    }
    return false;
}

void lua_quote(std::string& out, const uint8_t* text, int length) {
    out.push_back('"');
    for (int i = 0; i < length; ++i) {
        unsigned char bite = text[i];
        if (bite == '\\' || bite == '"') {
            out.push_back('\\');
            out.push_back(static_cast<char>(bite));
        } else if (bite == '\n') {
            out += "\\n";
        } else if (bite == '\r') {
            continue;
        } else if (bite >= 32) {
            out.push_back(static_cast<char>(bite));
        }
    }
    out.push_back('"');
}

bool read_counted(const uint8_t* data, int size, int& cursor, const uint8_t*& text, uint16_t& length) {
    if (cursor + 2 > size) {
        return false;
    }
    memcpy(&length, data + cursor, 2);
    cursor += 2;
    if (cursor + length > size) {
        return false;
    }
    text = data + cursor;
    cursor += length;
    return true;
}

bool read_perk(const uint8_t* data, int size, int& cursor) {
    if (cursor + 4 > size) {
        return false;
    }
    uint32_t flags = 0;
    memcpy(&flags, data + cursor, 4);
    cursor += 4;
    auto u8 = [&](uint32_t& out) {
        if (cursor >= size) {
            return false;
        }
        out = data[cursor++];
        return true;
    };
    auto u16 = [&](uint32_t& out) {
        if (cursor + 2 > size) {
            return false;
        }
        uint16_t value = 0;
        memcpy(&value, data + cursor, 2);
        cursor += 2;
        out = value;
        return true;
    };
    auto u32 = [&](uint32_t& out) {
        if (cursor + 4 > size) {
            return false;
        }
        memcpy(&out, data + cursor, 4);
        cursor += 4;
        return true;
    };
    uint32_t id = 0;
    uint32_t icon = 0;
    uint32_t req = 0;
    uint32_t group = 0;
    uint32_t info = 0;
    uint32_t cat = 0;
    uint32_t sort = 0;
    uint32_t perk_flags = 4;
    uint32_t options = 0;
    uint32_t unlock = 0;
    int x = 0x7f;
    int y = 0x7f;
    if (flags & 1u) {
        if (!u16(id)) return false;
    } else if (!u32(id)) {
        return false;
    }
    if (flags & 2u) {
        if (!u16(icon)) return false;
    } else if (!u32(icon)) {
        return false;
    }
    if (flags & 4u) {
        if (!u16(req)) return false;
    } else if (flags & 8u) {
        if (!u32(req)) return false;
    }
    if (flags & 0x10u) {
        if (!u16(group)) return false;
    } else if (flags & 0x20u) {
        if (!u32(group)) return false;
    }
    if ((flags & 0x40u) && !u32(info)) {
        return false;
    }
    if (flags & 0x80u) {
        if (!u32(cat)) return false;
    } else if (!u8(cat)) {
        return false;
    }
    if (flags & 0x100u) {
        if (!u32(sort)) return false;
    } else if (flags & 0x200u) {
        if (cursor >= size) return false;
        sort = static_cast<uint32_t>(static_cast<int8_t>(data[cursor++]));
    }
    uint32_t levels = 1;
    if (flags & 0x400u) {
        if (cursor >= size) return false;
        levels = data[cursor++];
    } else if (flags & 0x800u) {
        if (!u32(levels)) return false;
    }
    if (levels == 0) {
        levels = 1;
    } else if (levels > 40) {
        levels = 40;
    }
    if (flags & 0x1000u) {
        if (!u16(unlock)) return false;
    } else if (flags & 0x2000u) {
        if (!u32(unlock)) return false;
    }
    if (flags & 0x4000u) {
        if (!read_varint(data, size, cursor, perk_flags)) return false;
    }
    if (flags & 0x8000u) {
        if (!read_varint(data, size, cursor, options)) return false;
    }
    uint32_t extra = 0;
    if (flags & 0x10000u) {
        if (!read_varint(data, size, cursor, extra)) return false;
    }
    if (flags & 0x20000u) {
        if (cursor + 2 > size) return false;
        x = static_cast<int8_t>(data[cursor++]);
        y = static_cast<int8_t>(data[cursor++]);
    }
    if (cursor + 2 > size) {
        return false;
    }
    uint16_t mask = 0;
    memcpy(&mask, data + cursor, 2);
    cursor += 2;
    float words[16]{};
    bool present[16]{};
    for (int bit = 0; bit < 16; ++bit) {
        if ((mask & (1u << bit)) == 0) {
            continue;
        }
        if (cursor + 4 > size) {
            return false;
        }
        memcpy(&words[bit], data + cursor, 4);
        cursor += 4;
        present[bit] = true;
    }
    const uint8_t* name = nullptr;
    const uint8_t* desc = nullptr;
    uint16_t name_n = 0;
    uint16_t desc_n = 0;
    if (!read_counted(data, size, cursor, name, name_n) || !read_counted(data, size, cursor, desc, desc_n)) {
        return false;
    }
    if (id == 0) {
        return true;
    }
    std::string line = "PerkMgrPerks[";
    line += std::to_string(id);
    line += "]={id=";
    line += std::to_string(id);
    line += ",icon=";
    line += std::to_string(icon);
    line += ",name=";
    lua_quote(line, name, name_n);
    line += ",info=";
    line += std::to_string(info);
    line += ",req=";
    line += std::to_string(req);
    line += ",sg=";
    line += std::to_string(group);
    // ʕ •ᴥ•ʔ✿ "Balance: <class>" belongs on that class tab. Misc keeps the shared list. ✿ ʕ •ᴥ•ʔ
    if (name_n >= 8 && memcmp(name, "Balance:", 8) == 0 && req && (req & (req - 1u)) == 0u && req <= 1024u) {
        int class_id = 0;
        for (uint32_t bit = req; bit; bit >>= 1) {
            ++class_id;
        }
        cat = class_id;
    }
    line += ",flags=";
    line += std::to_string(perk_flags);
    line += ",options=";
    line += std::to_string(options);
    line += ",sort=";
    line += std::to_string(static_cast<int32_t>(sort));
    line += ",cat=";
    line += std::to_string(cat);
    line += ",desc=";
    lua_quote(line, desc, desc_n);
    line += ",x=";
    line += std::to_string(x);
    line += ",y=";
    line += std::to_string(y);
    line += ",unlock=";
    line += std::to_string(unlock);
    line += ",lvflags=";
    line += std::to_string(extra);
    line += ",levels={";
    auto append_num = [](std::string& out, float value) {
        float rounded = std::round(value);
        if (std::fabs(value - rounded) < 0.001f) {
            out += std::to_string(static_cast<int>(rounded));
            return;
        }
        char buf[32];
        snprintf(buf, sizeof(buf), "%.4g", value);
        out += buf;
    };
    auto rank_amount = [](float start, float end, bool has_start, bool has_end, uint32_t index, uint32_t count) {
        if (!has_start && !has_end) {
            return 0.0f;
        }
        if (!has_start) {
            start = end;
        }
        if (!has_end) {
            end = start;
        }
        if (count <= 1 || index == 0) {
            return start;
        }
        if (index + 1 >= count) {
            return end;
        }
        float t = static_cast<float>(index) / static_cast<float>(count - 1);
        return start + (end - start) * t;
    };
    for (uint32_t level = 0; level < levels; ++level) {
        if (level) {
            line += ',';
        }
        line += "{amounts={";
        for (int slot = 0; slot < 4; ++slot) {
            if (slot) {
                line += ',';
            }
            int bit = slot * 4;
            append_num(line, rank_amount(words[bit], words[bit + 1], present[bit], present[bit + 1], level, levels));
        }
        line += ",0},amounts2={";
        for (int slot = 0; slot < 4; ++slot) {
            if (slot) {
                line += ',';
            }
            int bit = slot * 4;
            bool has_start = present[bit + 2];
            bool has_end = present[bit + 3];
            float start = words[bit + 2];
            float end = words[bit + 3];
            if (!has_start && !has_end) {
                has_start = present[bit];
                has_end = present[bit + 1];
                start = words[bit];
                end = words[bit + 1];
            }
            append_num(line, rank_amount(start, end, has_start, has_end, level, levels));
        }
        line += ",0}}";
    }
    line += "}}\n";
    g_perk_lua += line;
    ++g_perk_count;
    return true;
}

bool perk_text_ok(const uint8_t* text, int length) {
    for (int i = 0; i < length; ++i) {
        unsigned char bite = text[i];
        if (bite < 32 && bite != '\n' && bite != '\r' && bite != '\t') {
            return false;
        }
    }
    return length > 0;
}

void lua_amount(std::string& out, float value) {
    float rounded = std::round(value);
    if (std::fabs(value - rounded) < 0.001f) {
        out += std::to_string(static_cast<int>(rounded));
        return;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "%.4g", value);
    out += buf;
}

// ʕ •ᴥ•ʔ✿ Set records follow the perk array. Tag bit 0x40 carries amount pairs for $0..$3. ✿ ʕ •ᴥ•ʔ
bool read_perk_sets(const uint8_t* data, int size, int cursor, int& end_at) {
    if (cursor + 4 > size) {
        return false;
    }
    uint32_t nsets = 0;
    memcpy(&nsets, data + cursor, 4);
    cursor += 4;
    if (nsets == 0 || nsets > 64) {
        return false;
    }
    std::string lua;
    for (uint32_t set_index = 0; set_index < nsets; ++set_index) {
        if (cursor + 3 > size) {
            return false;
        }
        uint8_t set_flags = data[cursor++];
        uint32_t set_id = 0;
        if (set_flags & 1u) {
            if (cursor >= size) {
                return false;
            }
            set_id = data[cursor++];
        } else if (set_flags & 2u) {
            if (cursor + 2 > size) {
                return false;
            }
            uint16_t wide = 0;
            memcpy(&wide, data + cursor, 2);
            cursor += 2;
            set_id = wide;
        } else {
            if (cursor + 4 > size) {
                return false;
            }
            memcpy(&set_id, data + cursor, 4);
            cursor += 4;
        }
        if (set_id == 0 || cursor >= size) {
            return false;
        }
        if (data[cursor] <= 32) {
            ++cursor;
        }
        auto bonus_header = [&](int pos, uint16_t& tag, uint8_t& count, uint16_t& desc_n, const uint8_t*& desc) {
            if (pos + 5 > size) {
                return false;
            }
            memcpy(&tag, data + pos, 2);
            count = data[pos + 2];
            memcpy(&desc_n, data + pos + 3, 2);
            if (tag == 0 || count == 0 || count > 16 || desc_n > 500 || pos + 5 + desc_n > size) {
                return false;
            }
            desc = data + pos + 5;
            return !desc_n || perk_text_ok(desc, desc_n);
        };
        auto name_at = [&](int pos) {
            auto one = [&](bool colored) {
                int at = pos + (colored ? 4 : 0);
                if (at + 2 > size) {
                    return false;
                }
                uint16_t length = 0;
                memcpy(&length, data + at, 2);
                return length >= 1 && length <= 48 && at + 2 + length <= size && perk_text_ok(data + at + 2, length);
            };
            return one(false) || one(true);
        };
        uint16_t perk_ids[32];
        int perk_count = 0;
        bool skipped = false;
        while (perk_count < 32 && cursor + 2 <= size) {
            uint16_t tag = 0;
            uint8_t count = 0;
            uint16_t desc_n = 0;
            const uint8_t* desc = nullptr;
            if (bonus_header(cursor, tag, count, desc_n, desc)) {
                break;
            }
            uint16_t perk_id = 0;
            memcpy(&perk_id, data + cursor, 2);
            if (perk_id >= 1 && perk_id <= 8000) {
                perk_ids[perk_count++] = perk_id;
                cursor += 2;
                continue;
            }
            if (!skipped) {
                skipped = true;
                ++cursor;
                continue;
            }
            return false;
        }
        if (perk_count == 0) {
            return false;
        }
        struct Bonus {
            uint8_t count;
            uint32_t perk;
            float amounts[4];
            float amounts2[4];
            const uint8_t* desc;
            uint16_t desc_n;
        };
        Bonus bonuses[16];
        int nbonus = 0;
        while (nbonus < 16) {
            uint16_t tag = 0;
            uint8_t count = 0;
            uint16_t desc_n = 0;
            const uint8_t* desc = nullptr;
            if (!bonus_header(cursor, tag, count, desc_n, desc)) {
                break;
            }
            int body = cursor + 5 + desc_n;
            int order[5];
            int norder = 0;
            auto push_order = [&](int pairs) {
                for (int i = 0; i < norder; ++i) {
                    if (order[i] == pairs) {
                        return;
                    }
                }
                order[norder++] = pairs;
            };
            if (tag & 0x40) {
                int slots = 1;
                for (int i = 0; i + 1 < desc_n; ++i) {
                    if (desc[i] == '$' && desc[i + 1] >= '0' && desc[i + 1] <= '3') {
                        int slot = desc[i + 1] - '0' + 1;
                        if (slot > slots) {
                            slots = slot;
                        }
                    }
                }
                push_order(slots);
                for (int pairs = 1; pairs <= 4; ++pairs) {
                    push_order(pairs);
                }
            } else {
                push_order(0);
            }
            int chosen = -1;
            uint32_t perk = 0;
            for (int i = 0; i < norder; ++i) {
                int end = body + order[i] * 8;
                if (end + 4 > size) {
                    continue;
                }
                uint32_t candidate = 0;
                memcpy(&candidate, data + end, 4);
                if (candidate < 1 || candidate > 30000) {
                    continue;
                }
                uint16_t next_tag = 0;
                uint8_t next_count = 0;
                uint16_t next_desc_n = 0;
                const uint8_t* next_desc = nullptr;
                if (bonus_header(end + 4, next_tag, next_count, next_desc_n, next_desc) || name_at(end + 4)) {
                    chosen = order[i];
                    perk = candidate;
                    break;
                }
            }
            if (chosen < 0) {
                break;
            }
            Bonus bonus{};
            bonus.count = count;
            bonus.perk = perk;
            bonus.desc = desc;
            bonus.desc_n = desc_n;
            for (int slot = 0; slot < chosen && slot < 4; ++slot) {
                memcpy(&bonus.amounts[slot], data + body + slot * 8, 4);
                memcpy(&bonus.amounts2[slot], data + body + slot * 8 + 4, 4);
            }
            bonuses[nbonus++] = bonus;
            cursor = body + chosen * 8 + 4;
        }
        if (nbonus == 0) {
            return false;
        }
        auto take_name = [&](bool colored, uint32_t& color, const uint8_t*& name, uint16_t& name_n, int& next) {
            int pos = cursor;
            color = 0;
            if (colored) {
                if (pos + 4 > size) {
                    return false;
                }
                memcpy(&color, data + pos, 4);
                pos += 4;
            }
            if (pos + 2 > size) {
                return false;
            }
            memcpy(&name_n, data + pos, 2);
            if (name_n < 1 || name_n > 48 || pos + 2 + name_n > size || !perk_text_ok(data + pos + 2, name_n)) {
                return false;
            }
            name = data + pos + 2;
            next = pos + 2 + name_n;
            return true;
        };
        uint32_t color = 0;
        const uint8_t* name = nullptr;
        uint16_t name_n = 0;
        int next = cursor;
        if (!take_name(false, color, name, name_n, next) && !take_name(true, color, name, name_n, next)) {
            return false;
        }
        cursor = next;
        lua += "PerkMgrSets[";
        lua += std::to_string(set_id);
        lua += "]={id=";
        lua += std::to_string(set_id);
        lua += ",name=";
        lua_quote(lua, name, name_n);
        lua += ",color=";
        lua += std::to_string(color);
        lua += ",perks={";
        for (int i = 0; i < perk_count; ++i) {
            if (i) {
                lua += ',';
            }
            lua += std::to_string(perk_ids[i]);
        }
        lua += "},bonus={";
        for (int i = 0; i < nbonus; ++i) {
            if (i) {
                lua += ',';
            }
            const Bonus& bonus = bonuses[i];
            lua += "{count=";
            lua += std::to_string(bonus.count);
            lua += ",perkid=";
            lua += std::to_string(bonus.perk);
            lua += ",desc=";
            lua_quote(lua, bonus.desc, bonus.desc_n);
            lua += ",amounts={";
            for (int slot = 0; slot < 4; ++slot) {
                if (slot) {
                    lua += ',';
                }
                lua_amount(lua, bonus.amounts[slot]);
            }
            lua += ",0},amounts2={";
            for (int slot = 0; slot < 4; ++slot) {
                if (slot) {
                    lua += ',';
                }
                lua_amount(lua, bonus.amounts2[slot]);
            }
            lua += ",0}}";
        }
        lua += "}}\n";
        log_line("perk set %u %.*s bonuses=%d perks=%d", set_id, static_cast<int>(name_n), reinterpret_cast<const char*>(name), nbonus, perk_count);
    }
    g_perk_lua += lua;
    g_perk_set_count += static_cast<int>(nsets);
    end_at = cursor;
    return true;
}

bool skip_perk_task(const uint8_t* data, int size, int& cursor) {
    if (cursor + 2 > size) {
        return false;
    }
    uint16_t flags = 0;
    memcpy(&flags, data + cursor, 2);
    cursor += 2;
    auto skip = [&](int bytes) {
        if (cursor + bytes > size) {
            return false;
        }
        cursor += bytes;
        return true;
    };
    if (flags & 1u) {
        if (!skip(2)) return false;
    } else if (!skip(4)) {
        return false;
    }
    if ((flags & 8u) == 0) {
        if (!skip(flags & 0x10u ? 1 : 4)) return false;
    }
    if ((flags & 0x20u) && !skip(4)) return false;
    if ((flags & 0x40u) && !skip(4)) return false;
    if ((flags & 0x80u) && !skip(4)) return false;
    if (flags & 0x100u) {
        if (!skip(1)) return false;
    } else if ((flags & 0x200u) && !skip(4)) {
        return false;
    }
    if (flags & 0x400u) {
        for (int i = 0; i < 5 && cursor < size; ++i) {
            uint8_t bite = data[cursor++];
            if ((bite & 0x80) == 0) {
                break;
            }
        }
    }
    if (cursor + 2 > size) {
        return false;
    }
    uint16_t length = 0;
    memcpy(&length, data + cursor, 2);
    cursor += 2;
    if (length > 800 || cursor + length > size) {
        return false;
    }
    cursor += length;
    return true;
}

bool read_perk_synergies(const uint8_t* data, int size, int cursor) {
    if (cursor + 4 > size) {
        return false;
    }
    uint32_t tasks = 0;
    memcpy(&tasks, data + cursor, 4);
    cursor += 4;
    if (tasks > 4000) {
        return false;
    }
    for (uint32_t i = 0; i < tasks; ++i) {
        if (!skip_perk_task(data, size, cursor)) {
            return false;
        }
    }
    if (cursor + 4 > size) {
        return false;
    }
    uint32_t names = 0;
    memcpy(&names, data + cursor, 4);
    cursor += 4;
    if (names > 4000) {
        return false;
    }
    for (uint32_t i = 0; i < names; ++i) {
        if (cursor + 3 > size) {
            return false;
        }
        ++cursor;
        uint16_t length = 0;
        memcpy(&length, data + cursor, 2);
        cursor += 2;
        if (length > 200 || cursor + length + 1 > size) {
            return false;
        }
        cursor += length + 1;
    }
    if (cursor + 8 > size) {
        return false;
    }
    uint32_t pad = 0;
    uint32_t count = 0;
    memcpy(&pad, data + cursor, 4);
    memcpy(&count, data + cursor + 4, 4);
    cursor += 8;
    if (pad != 0 || count > 4000) {
        return false;
    }
    std::string lua;
    for (uint32_t i = 0; i < count; ++i) {
        if (cursor + 2 > size) {
            return false;
        }
        uint16_t flags = 0;
        memcpy(&flags, data + cursor, 2);
        cursor += 2;
        uint32_t raw = 0;
        for (int step = 0; step < 5 && cursor < size; ++step) {
            uint8_t bite = data[cursor++];
            raw = (raw << 7) | (bite & 0x7fu);
            if ((bite & 0x80) == 0) {
                break;
            }
        }
        int32_t sid = static_cast<int32_t>(raw >> 1);
        if (raw & 1u) {
            sid = -sid;
        }
        if (cursor + 2 > size) {
            return false;
        }
        uint8_t index = data[cursor++];
        uint8_t cl1 = data[cursor++];
        uint8_t cl2 = 0;
        uint16_t clm = 0;
        uint32_t text_id = 0;
        const uint8_t* text = nullptr;
        uint16_t text_n = 0;
        uint32_t entry_flags = 4;
        uint8_t pieces = 0;
        float number = 0;
        uint32_t spells[16];
        int nspells = 0;
        uint32_t qv = 0;
        float qv2 = 0;
        if (flags & 1u) {
            if (cursor >= size) return false;
            cl2 = data[cursor++];
        }
        if (flags & 2u) {
            if (cursor + 2 > size) return false;
            memcpy(&clm, data + cursor, 2);
            cursor += 2;
        }
        if (flags & 4u) {
            for (int step = 0; step < 5 && cursor < size; ++step) {
                uint8_t bite = data[cursor++];
                text_id = (text_id << 7) | (bite & 0x7fu);
                if ((bite & 0x80) == 0) break;
            }
        }
        if (flags & 8u) {
            if (cursor + 2 > size) return false;
            memcpy(&text_n, data + cursor, 2);
            cursor += 2;
            if (text_n > 800 || cursor + text_n > size) return false;
            text = data + cursor;
            cursor += text_n;
        }
        if (flags & 0x10u) {
            if (cursor + 4 > size) return false;
            memcpy(&entry_flags, data + cursor, 4);
            cursor += 4;
        }
        if (flags & 0x20u) {
            if (cursor >= size) return false;
            pieces = data[cursor++];
        }
        if (flags & 0x40u) {
            if (cursor + 4 > size) return false;
            memcpy(&number, data + cursor, 4);
            cursor += 4;
        }
        if (flags & 0x80u) {
            if (cursor >= size) return false;
            nspells = data[cursor++];
            if (nspells > 16 || cursor + nspells * 4 > size) return false;
            for (int spell = 0; spell < nspells; ++spell) {
                memcpy(&spells[spell], data + cursor, 4);
                cursor += 4;
            }
        }
        if (flags & 0x100u) {
            if (cursor + 4 > size) return false;
            memcpy(&qv, data + cursor, 4);
            cursor += 4;
        }
        if (flags & 0x200u) {
            if (cursor + 4 > size) return false;
            memcpy(&qv2, data + cursor, 4);
            cursor += 4;
        }
        lua += "_addPerkSynergy(";
        lua += std::to_string(sid);
        lua += ",{id=";
        lua += std::to_string(sid);
        lua += ",index=";
        lua += std::to_string(index);
        lua += ",cl1=";
        lua += std::to_string(cl1);
        if (cl2) {
            lua += ",cl2=";
            lua += std::to_string(cl2);
        }
        if (clm) {
            lua += ",clm=";
            lua += std::to_string(clm);
        }
        lua += ",flags=";
        lua += std::to_string(entry_flags);
        if (text && text_n) {
            lua += ",t=";
            lua_quote(lua, text, text_n);
        } else if (text_id) {
            lua += ",t=";
            lua += std::to_string(text_id);
        }
        if (pieces) {
            lua += ",s=";
            lua += std::to_string(pieces);
        }
        if (number != 0) {
            lua += ",num=";
            lua_amount(lua, number);
        }
        if (nspells) {
            lua += ",sp={";
            for (int spell = 0; spell < nspells; ++spell) {
                if (spell) lua += ',';
                lua += std::to_string(spells[spell]);
            }
            lua += '}';
        }
        if (qv) {
            lua += ",qv=";
            lua += std::to_string(qv);
        }
        if (qv2 != 0) {
            lua += ",qv2=";
            lua_amount(lua, qv2);
        }
        lua += "})\n";
    }
    g_synergy_lua += lua;
    g_synergy_count += static_cast<int>(count);
    return true;
}

void spell_copy_from(SpellTable& table, int dest, int source) {
    if (dest <= 0 || source <= 0 || dest == source || dest >= kSpellIdLimit || source >= kSpellIdLimit) {
        return;
    }
    uint8_t* from = spell_plain(table, source);
    uint8_t* row = spell_plain(table, dest);
    if (!from || !row) {
        return;
    }
    memcpy(row, from, kSpellBytes);
    int32_t id = dest;
    memcpy(row, &id, 4);
    spell_store(table, dest);
}

void spell_put_i32(SpellTable& table, int id, int at, int32_t value) {
    uint8_t* row = spell_plain(table, id);
    if (!row || at < 0 || at + 4 > static_cast<int>(kSpellBytes)) {
        return;
    }
    memcpy(row + at, &value, 4);
    spell_store(table, id);
}

bool take_text(const uint8_t* data, int size, int& cursor, const uint8_t*& text, uint16_t& length) {
    if (cursor + 2 > size) {
        return false;
    }
    memcpy(&length, data + cursor, 2);
    if (cursor + 2 + length > size) {
        return false;
    }
    text = data + cursor + 2;
    cursor += 2 + length;
    return true;
}

void spell_copy_icon(SpellTable& table, int id, int source) {
    if (source < table.min_id || source > table.max_id || !table.by_id) {
        return;
    }
    if (table.plain.find(source) == table.plain.end() && !table.by_id[source - table.min_id]) {
        return;
    }
    uint8_t* from = spell_plain(table, source);
    uint8_t* row = spell_plain(table, id);
    if (!from || !row) {
        return;
    }
    memcpy(row + kSpellIconAt, from + kSpellIconAt, 8);
    spell_store(table, id);
}

bool spell_mod_skip(const uint8_t* data, int size, int& cursor, int type) {
    auto fixed = [&](int bytes) {
        if (cursor + bytes > size) {
            return false;
        }
        cursor += bytes;
        return true;
    };
    auto chars = [&]() {
        if (cursor + 2 > size) {
            return false;
        }
        uint16_t length = 0;
        memcpy(&length, data + cursor, 2);
        if (cursor + 2 + length > size) {
            return false;
        }
        cursor += 2 + length;
        return true;
    };
    if (type == 1 || type == 4 || type == 9 || type == 10 || type == 23 || type == 24 || type == 26 || type == 28
        || type == 31 || type == 32 || type == 40 || type == 41 || type == 47 || type == 53 || type == 54 || type == 55) {
        return fixed(8);
    }
    if (type == 37) {
        return true;
    }
    if (type == 38) {
        return fixed(9);
    }
    if (type == 52) {
        return fixed(7);
    }
    if (type == 11 || type == 18 || type == 21 || type == 27 || type == 30 || type == 33) {
        return fixed(9);
    }
    if (type == 14 || type == 19 || type == 35) {
        return fixed(12);
    }
    if (type == 17 || type == 20 || type == 25) {
        return fixed(13);
    }
    if (type == 8 || type == 13) {
        return fixed(16);
    }
    if (type == 12) {
        return fixed(17);
    }
    if (type == 16) {
        return fixed(10);
    }
    if (type == 39) {
        return fixed(5);
    }
    if (type == 3) {
        if (!fixed(12)) {
            return false;
        }
        return chars() && chars();
    }
    if (type == 34) {
        if (cursor >= size) {
            return false;
        }
        int count = data[cursor++];
        return fixed(count * 12);
    }
    if (type == 2) {
        return fixed(4) && chars();
    }
    if (type == 29) {
        int count = 0;
        int shift = 0;
        bool counted = false;
        while (cursor < size && shift < 28) {
            uint8_t bite = data[cursor++];
            count |= (bite & 0x7f) << shift;
            if ((bite & 0x80) == 0) {
                counted = true;
                break;
            }
            shift += 7;
        }
        return counted && fixed(count * 4);
    }
    if (type == 36) {
        if (cursor + 8 > size) {
            return false;
        }
        uint32_t flags = 0;
        memcpy(&flags, data + cursor + 4, 4);
        cursor += 8;
        static const uint32_t low_bits[] = {1u, 2u, 4u, 8u, 0x10u};
        for (uint32_t bit : low_bits) {
            if ((flags & bit) && !fixed(4)) {
                return false;
            }
        }
        for (uint32_t bit = 0x20; bit <= 0x200; bit <<= 1) {
            if ((flags & bit) && !fixed(8)) {
                return false;
            }
        }
        return true;
    }
    if (type == 44) {
        if (cursor + 6 > size) {
            return false;
        }
        uint16_t bits = 0;
        memcpy(&bits, data + cursor, 2);
        cursor += 6;
        for (int bit = 0; bit < 6; ++bit) {
            if ((bits & (1u << bit)) && !fixed(4)) {
                return false;
            }
        }
        return true;
    }
    if (type == 45 || type == 46) {
        if (cursor + 2 > size) {
            return false;
        }
        uint16_t count = 0;
        memcpy(&count, data + cursor, 2);
        cursor += 2;
        return fixed(static_cast<int>(count) * 4);
    }
    if (type == 50) {
        return fixed(5) && chars();
    }
    return false;
}

void install_skill_abilities() {
    if (g_skill_adds.empty()) {
        return;
    }
    void* found = nullptr;
    if (!syn_resolve("g_skillLineAbilityDB", &found) || !found) {
        log_line("skill ability db missing");
        return;
    }
    auto* db = static_cast<uint8_t*>(found);
    int32_t count = 0;
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&count, db + 0x0c, 4);
    memcpy(&max_id, db + 0x10, 4);
    memcpy(&min_id, db + 0x14, 4);
    uint8_t* records = nullptr;
    uint8_t** by_id = nullptr;
    memcpy(&records, db + 0x28, sizeof(records));
    memcpy(&by_id, db + 0x30, sizeof(by_id));
    if (!records || !by_id || count <= 0 || max_id < min_id) {
        log_line("skill ability db empty");
        return;
    }
    int extra = static_cast<int>(g_skill_adds.size());
    int32_t new_max = max_id + extra;
    auto* grown = new uint8_t[static_cast<size_t>(count + extra) * 0x38];
    memcpy(grown, records, static_cast<size_t>(count) * 0x38);
    size_t old_n = static_cast<size_t>(max_id - min_id + 1);
    size_t new_n = static_cast<size_t>(new_max - min_id + 1);
    uint8_t** grown_by = new uint8_t*[new_n]();
    for (size_t i = 0; i < old_n; ++i) {
        if (!by_id[i]) {
            continue;
        }
        ptrdiff_t delta = by_id[i] - records;
        if (delta >= 0 && static_cast<size_t>(delta) < static_cast<size_t>(count) * 0x38) {
            grown_by[i] = grown + delta;
        }
    }
    int32_t next = max_id;
    int32_t slot = count;
    for (const SkillAdd& add : g_skill_adds) {
        ++next;
        uint8_t* row = grown + static_cast<size_t>(slot) * 0x38;
        memset(row, 0, 0x38);
        int32_t id = next;
        int32_t skill = add.skill;
        int32_t spell = static_cast<int32_t>(add.spell);
        int32_t mask = add.mask;
        int32_t method = 1;
        memcpy(row + 0x00, &id, 4);
        memcpy(row + 0x04, &skill, 4);
        memcpy(row + 0x08, &spell, 4);
        memcpy(row + 0x10, &mask, 4);
        memcpy(row + 0x24, &method, 4);
        grown_by[static_cast<size_t>(next - min_id)] = row;
        ++slot;
    }
    int32_t total = count + extra;
    memcpy(db + 0x28, &grown, sizeof(grown));
    memcpy(db + 0x30, &grown_by, sizeof(grown_by));
    memcpy(db + 0x0c, &total, 4);
    memcpy(db + 0x10, &new_max, 4);
    log_line("skill abilities added %d", extra);
#if defined(SYNASTRIA_EMBED)
    SpellTableResetSkillLines();
#endif
}

void publish_spell_records() {
    static bool done = false;
    if (done) {
        return;
    }
    SpellTable& table = spell_table();
    if (!table.base || !table.by_id) {
        return;
    }
    done = true;
    g_skill_adds.clear();
    g_perk_lua.clear();
    g_synergy_lua.clear();
    g_perk_count = 0;
    g_perk_set_count = 0;
    g_synergy_count = 0;
    char path[1024];
    snprintf(path, sizeof(path), "%s/WoWExt.bin", g_root);
    FILE* file = fopen(path, "rb");
    if (!file) {
        return;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    std::vector<uint8_t> data(static_cast<size_t>(length > 0 ? length : 0));
    if (length <= 12 || fread(data.data(), 1, data.size(), file) != data.size()) {
        fclose(file);
        return;
    }
    fclose(file);
    uint32_t count = 0;
    memcpy(&count, data.data() + 8, 4);
    if (count >= 0x1000) {
        return;
    }
    std::vector<char> plain(1 << 22);
    size_t offset = 12;
    int names = 0;
    int described = 0;
    int icons = 0;
    int copies = 0;
    int auras = 0;
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t bytes = 0;
        if (offset + 24 > data.size()) {
            break;
        }
        offset += 20;
        memcpy(&bytes, data.data() + offset, 4);
        offset += 4;
        if (offset + bytes > data.size()) {
            break;
        }
        int plain_size = LZ4_decompress_safe(reinterpret_cast<const char*>(data.data() + offset), plain.data(),
                                              static_cast<int>(bytes), static_cast<int>(plain.size()));
        offset += bytes;
        if (plain_size < 8) {
            continue;
        }
        const uint8_t* cursor = reinterpret_cast<const uint8_t*>(plain.data());
        int at = 0;
        while (at + 2 <= plain_size) {
            ++at;
            int type = 0;
            int shift = 0;
            bool typed = false;
            while (at < plain_size && shift < 28) {
                uint8_t bite = cursor[at++];
                type |= (bite & 0x7f) << shift;
                if ((bite & 0x80) == 0) {
                    typed = true;
                    break;
                }
                shift += 7;
            }
            if (!typed || type < 1 || type > 57) {
                break;
            }
            if (type == 1 || type == 24) {
                if (at + 8 > plain_size) {
                    break;
                }
                uint32_t source = 0;
                uint32_t dest = 0;
                memcpy(&source, cursor + at, 4);
                memcpy(&dest, cursor + at + 4, 4);
                at += 8;
                spell_copy_from(table, static_cast<int>(dest), static_cast<int>(source));
                if (type == 24) {
                    spell_put_i32(table, static_cast<int>(dest), 0x134, 0);
                    spell_put_i32(table, static_cast<int>(dest), 0x138, 0);
                    spell_put_i32(table, static_cast<int>(dest), 0x13c, 0);
                    uint8_t* row = spell_plain(table, static_cast<int>(dest));
                    if (row) {
                        char* blank = nullptr;
                        memcpy(row + kSpellNameAt + static_cast<int>(sizeof(char*)), &blank, sizeof(blank));
                        spell_store(table, static_cast<int>(dest));
                    }
                }
                ++copies;
                continue;
            }
            if (type == 3) {
                if (at + 12 > plain_size) {
                    break;
                }
                uint32_t spell = 0;
                uint32_t flags = 0;
                uint32_t icon_from = 0;
                memcpy(&spell, cursor + at, 4);
                memcpy(&flags, cursor + at + 4, 4);
                memcpy(&icon_from, cursor + at + 8, 4);
                at += 12;
                const uint8_t* name = nullptr;
                const uint8_t* aura = nullptr;
                uint16_t name_n = 0;
                uint16_t aura_n = 0;
                if (!take_text(cursor, plain_size, at, name, name_n) || !take_text(cursor, plain_size, at, aura, aura_n)) {
                    break;
                }
                if (spell > 0 && spell < static_cast<uint32_t>(kSpellIdLimit)) {
                    spell_copy_from(table, static_cast<int>(spell), 0x659d);
                    int32_t attr = static_cast<int32_t>(0x89000000);
                    if (flags & 1u) {
                        attr = static_cast<int32_t>(0x8d000000);
                    }
                    if ((flags & 2u) == 0) {
                        attr |= 0x100;
                    }
                    if ((flags & 4u) == 0) {
                        attr |= 0x20000000;
                    }
                    spell_put_i32(table, static_cast<int>(spell), 0x10, attr);
                    if (name_n > 0) {
                        spell_set_text(table, static_cast<int>(spell), kSpellNameAt, name, name_n);
                    }
                    if (aura_n > 0) {
                        spell_set_text(table, static_cast<int>(spell), kSpellAuraAt, aura, aura_n);
                    }
                    if (icon_from > 0 && icon_from < static_cast<uint32_t>(kSpellIdLimit)) {
                        spell_copy_icon(table, static_cast<int>(spell), static_cast<int>(icon_from));
                    }
                    ++auras;
                }
                continue;
            }
            if (type == 13) {
                if (at + 16 > plain_size) {
                    break;
                }
                uint32_t spell = 0;
                int32_t recovery = 0;
                int32_t category = 0;
                int32_t category_recovery = 0;
                memcpy(&spell, cursor + at, 4);
                memcpy(&recovery, cursor + at + 4, 4);
                memcpy(&category, cursor + at + 8, 4);
                memcpy(&category_recovery, cursor + at + 12, 4);
                at += 16;
                if (spell > 0 && spell < static_cast<uint32_t>(kSpellIdLimit)) {
                    int id = static_cast<int>(spell);
                    if (recovery >= 0) {
                        spell_put_i32(table, id, 0x74, recovery);
                    }
                    if (category >= 0) {
                        spell_put_i32(table, id, 0x04, category);
                    }
                    if (category_recovery >= 0) {
                        spell_put_i32(table, id, 0x78, category_recovery);
                    }
                }
                continue;
            }
            if (type == 14) {
                if (at + 12 > plain_size) {
                    break;
                }
                uint32_t spell = 0;
                uint32_t field = 0;
                uint32_t value = 0;
                memcpy(&spell, cursor + at, 4);
                memcpy(&field, cursor + at + 4, 4);
                memcpy(&value, cursor + at + 8, 4);
                at += 12;
                int slot = spell_field_at(field);
                if (spell > 0 && spell < static_cast<uint32_t>(kSpellIdLimit) && slot >= 0 && slot + 4 <= static_cast<int>(kSpellBytes)) {
                    spell_put_i32(table, static_cast<int>(spell), slot, static_cast<int32_t>(value));
                }
                continue;
            }
            if (type == 16) {
                if (at + 10 > plain_size) {
                    break;
                }
                uint32_t spell = 0;
                uint32_t value = 0;
                uint8_t op = cursor[at + 4];
                uint8_t index = cursor[at + 5];
                memcpy(&spell, cursor + at, 4);
                memcpy(&value, cursor + at + 6, 4);
                at += 10;
                int slot = 0x10 + static_cast<int>(index) * 4;
                if (spell > 0 && spell < static_cast<uint32_t>(kSpellIdLimit) && index < 8) {
                    int id = static_cast<int>(spell);
                    uint8_t* row = spell_plain(table, id);
                    int32_t current = 0;
                    if (row) {
                        memcpy(&current, row + slot, 4);
                    }
                    if (op == 0) {
                        current = static_cast<int32_t>(value);
                    } else if (op == 1) {
                        current |= static_cast<int32_t>(value);
                    } else if (op == 2) {
                        current &= ~static_cast<int32_t>(value);
                    }
                    spell_put_i32(table, id, slot, current);
                }
                continue;
            }
            if (type == 15) {
                if (at + 4 > plain_size) {
                    break;
                }
                uint32_t perk_count = 0;
                memcpy(&perk_count, cursor + at, 4);
                at += 4;
                bool perks_ok = true;
                for (uint32_t perk = 0; perk < perk_count; ++perk) {
                    if (!read_perk(cursor, plain_size, at)) {
                        perks_ok = false;
                        break;
                    }
                }
                if (!perks_ok) {
                    break;
                }
                int after_sets = at;
                if (read_perk_sets(cursor, plain_size, at, after_sets)) {
                    read_perk_synergies(cursor, plain_size, after_sets);
                }
                at = plain_size;
                continue;
            }
            if (type == 11) {
                if (at + 9 > plain_size) {
                    break;
                }
                uint32_t spell = 0;
                uint32_t effect = 0;
                uint8_t index = cursor[at + 4];
                memcpy(&spell, cursor + at, 4);
                memcpy(&effect, cursor + at + 5, 4);
                at += 9;
                if (spell > 0 && spell < static_cast<uint32_t>(kSpellIdLimit) && index < 3) {
                    spell_put_i32(table, static_cast<int>(spell), 0x11c + static_cast<int>(index) * 4, static_cast<int32_t>(effect));
                }
                continue;
            }
            if (type == 26) {
                if (at + 8 > plain_size) {
                    break;
                }
                uint16_t skill = 0;
                uint32_t spell = 0;
                uint16_t mask = 0;
                memcpy(&skill, cursor + at, 2);
                memcpy(&spell, cursor + at + 2, 4);
                memcpy(&mask, cursor + at + 6, 2);
                at += 8;
                if (spell > 0 && skill > 0) {
                    g_skill_adds.push_back(SkillAdd{skill, spell, mask});
                }
                continue;
            }
            if (type == 5 || type == 6 || type == 7) {
                if (at + 6 > plain_size) {
                    break;
                }
                uint32_t spell = 0;
                uint16_t span = 0;
                memcpy(&spell, cursor + at, 4);
                memcpy(&span, cursor + at + 4, 2);
                at += 6;
                if (at + span > plain_size) {
                    break;
                }
                // ʕ •ᴥ•ʔ✿ Type 6 is the description. The 64-bit pointer is 8 bytes past the 32-bit slot. ✿ ʕ •ᴥ•ʔ
                if (spell > 0 && spell < static_cast<uint32_t>(kSpellIdLimit) && span > 0) {
                    int field = type == 5 ? kSpellNameAt : type == 6 ? kSpellDescAt : kSpellAuraAt;
                    spell_set_text(table, static_cast<int>(spell), field, cursor + at, span);
                    if (type == 5) {
                        ++names;
                    } else {
                        ++described;
                    }
                }
                at += span;
                continue;
            }
            if (type == 10) {
                if (at + 8 > plain_size) {
                    break;
                }
                uint32_t spell = 0;
                uint32_t source = 0;
                memcpy(&spell, cursor + at, 4);
                memcpy(&source, cursor + at + 4, 4);
                at += 8;
                if (spell > 0 && spell < static_cast<uint32_t>(kSpellIdLimit) && source < static_cast<uint32_t>(kSpellIdLimit)) {
                    spell_copy_icon(table, static_cast<int>(spell), static_cast<int>(source));
                    ++icons;
                }
                continue;
            }
            if (!spell_mod_skip(cursor, plain_size, at, type)) {
                break;
            }
        }
    }
    install_skill_abilities();
    for (int id : {80100, 80189}) {
        uint8_t* row = spell_plain(table, id);
        if (!row) {
            continue;
        }
        auto i32 = [&](int at) {
            int32_t value = 0;
            memcpy(&value, row + at, 4);
            return value;
        };
        char* spell_name = nullptr;
        memcpy(&spell_name, row + kSpellNameAt, sizeof(spell_name));
        log_line("spell %d %s attr=%#x ex=%#x cat=%d rec=%d crec=%d eff=%d icon=%d active=%d",
                 id, spell_name ? spell_name : "", i32(0x10), i32(0x14), i32(0x04), i32(0x74), i32(0x78),
                 i32(0x11c), i32(0x214), i32(0x218));
    }
    log_line("spell db names=%d text=%d icons=%d copies=%d auras=%d skills=%zu perks=%d sets=%d synergies=%d",
             names, described, icons, copies, auras, g_skill_adds.size(), g_perk_count, g_perk_set_count, g_synergy_count);
}

void publish_item_records() {
    void* found = nullptr;
    if (!syn_resolve("g_itemDB", &found) || !found) {
        log_line("item db missing");
        return;
    }
    auto* table = static_cast<uint8_t*>(found);
    int32_t max_id = 0;
    int32_t min_id = 0;
    int32_t count = 0;
    memcpy(&count, table + 0x0c, 4);
    memcpy(&max_id, table + 0x10, 4);
    memcpy(&min_id, table + 0x14, 4);
    void** by_id = nullptr;
    memcpy(&by_id, table + 0x30, sizeof(by_id));
    if (!by_id || max_id < min_id) {
        log_line("item db empty max=%d min=%d", max_id, min_id);
        return;
    }
    int need_max = max_id;
    int limit = static_cast<int>(g_items.size());
    for (int id = max_id + 1; id < limit; ++id) {
        if (g_items[static_cast<size_t>(id)].on) {
            need_max = id;
        }
    }
    if (need_max > max_id) {
        size_t grown_n = static_cast<size_t>(need_max - min_id + 1);
        size_t old_n = static_cast<size_t>(max_id - min_id + 1);
        void** grown = new void*[grown_n]();
        memcpy(grown, by_id, old_n * sizeof(void*));
        by_id = grown;
        memcpy(table + 0x30, &by_id, sizeof(by_id));
        memcpy(table + 0x10, &need_max, 4);
        max_id = need_max;
    }
    int added = 0;
    int written = 0;
    for (int id = min_id; id <= max_id && id < limit; ++id) {
        const ItemRow& row = g_items[static_cast<size_t>(id)];
        if (!row.on) {
            continue;
        }
        auto* rec = static_cast<int32_t*>(by_id[id - min_id]);
        if (!rec) {
            rec = new int32_t[8]();
            by_id[id - min_id] = rec;
            ++count;
            ++added;
        }
        rec[0] = id;
        rec[1] = static_cast<int8_t>(row.klass);
        rec[2] = static_cast<int8_t>(row.sub);
        rec[3] = static_cast<int8_t>(row.sound);
        rec[4] = static_cast<int8_t>(row.material);
        rec[5] = static_cast<int32_t>(row.display);
        rec[6] = static_cast<int8_t>(row.inv);
        rec[7] = static_cast<int8_t>(row.sheath);
        ++written;
    }
    memcpy(table + 0x0c, &count, 4);
    log_line("item db wrote %d added %d max=%d", written, added, max_id);
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

bool natives_visible() {
    if (!g_natives_ready || !g_context || !g_get_field || !g_type || !g_set_top) {
        return false;
    }
    void* state = g_context();
    if (!state) {
        return false;
    }
    g_get_field(state, kLuaGlobals, "CMCGetClassCount");
    int kind = g_type(state, -1);
    g_set_top(state, -2);
    return kind == 6;
}

void syn_features_before() {
    publish_spell_records();
    // ʕ •ᴥ•ʔ✿ The constructor runs before these vectors exist, so the file load waits for idle. ✿ ʕ •ᴥ•ʔ
    if (!g_items_loaded && g_root[0]) {
        g_items_loaded = true;
        syn_features_load(g_root);
    }
    if (!g_register && !bind_lua()) {
        return;
    }
    if (g_natives_ready && !natives_visible()) {
        g_natives_ready = false;
        g_scripts_ready = false;
        g_script_wait = 0;
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
    publish_item_records();
    publish_spell_records();
}

void syn_features_harvest() {
    harvest_characters();
}

bool syn_features_scripts_done() {
    return g_scripts_ready;
}

int syn_features_class_count() {
    return g_class_count;
}

int syn_features_class_id(int index) {
    if (index < 0 || index >= g_class_count) {
        return 0;
    }
    return g_classes[index].id;
}

void syn_features_after() {
    if (g_scripts_ready) {
        poll_spell_click();
        return;
    }
    if (!g_natives_ready) {
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

    uint32_t uleb() {
        uint32_t value = 0;
        int shift = 0;
        while (shift < 28 && ok) {
            uint8_t bite = u8();
            value |= static_cast<uint32_t>(bite & 0x7f) << shift;
            if ((bite & 0x80) == 0) {
                break;
            }
            shift += 7;
        }
        return value;
    }

    // ʕ •ᴥ•ʔ✿ Game-data blobs store the high 7 bits first. A normal varint swaps every multi-byte key. ✿ ʕ •ᴥ•ʔ
    uint32_t be7() {
        uint32_t value = 0;
        for (int i = 0; i < 5 && ok; ++i) {
            uint8_t bite = u8();
            if (!ok) {
                return 0;
            }
            value = (value << 7) | (bite & 0x7fu);
            if ((bite & 0x80) == 0) {
                break;
            }
        }
        return value;
    }
};

struct DataPair {
    uint32_t key = 0;
    uint64_t bits = 0;
};

void clear_cat(int cat) {
    g_game.erase(cat);
}

void clear_nums(int cat) {
    auto found = g_game.find(cat);
    if (found != g_game.end()) {
        found->second.nums.clear();
    }
}

uint32_t trunc_num(double value) {
    return static_cast<uint32_t>(static_cast<int64_t>(value));
}

bool decode_pairs(PktReader& in, std::vector<DataPair>& out) {
    uint16_t header = in.u16();
    if (!in.ok || header == 0) {
        return in.ok;
    }
    int key_fmt = header & 0xff;
    int value_fmt = header >> 8;
    std::vector<uint32_t> keys;
    if (key_fmt == 1) {
        uint32_t count = in.u32();
        for (uint32_t i = 0; i < count && in.ok; ++i) {
            keys.push_back(in.u32());
        }
    } else if (key_fmt == 2) {
        uint32_t acc = 0;
        while (in.ok) {
            uint32_t delta = in.be7();
            if (!in.ok || delta == 0) {
                break;
            }
            acc += delta;
            keys.push_back(acc);
        }
    } else if (key_fmt == 5) {
        keys.push_back(in.u32());
    } else if (key_fmt == 3) {
        uint32_t base = in.u32();
        uint32_t bits = in.u32();
        uint32_t nbytes = (bits + 7) / 8;
        for (uint32_t i = 0; i < nbytes && in.ok; ++i) {
            uint8_t bite = in.u8();
            for (int b = 0; b < 8; ++b) {
                if (bite & (1 << b)) {
                    keys.push_back(base + i * 8 + static_cast<uint32_t>(b));
                }
            }
        }
    } else if (key_fmt == 4) {
        uint32_t base = in.u32();
        uint32_t span = in.u32();
        if (!in.ok) {
            return false;
        }
        if (span != 0) {
            uint32_t groups = span / 64 + (span % 64 ? 1 : 0);
            std::vector<uint8_t> words(groups);
            if (!in.take(words.data(), groups)) {
                return false;
            }
            for (uint32_t group = 0; group < groups; ++group) {
                uint8_t word = words[group];
                if (!word) {
                    continue;
                }
                for (int bit = 0; bit < 8; ++bit) {
                    if ((word & (1 << bit)) == 0) {
                        continue;
                    }
                    uint8_t sub = in.u8();
                    for (int sub_bit = 0; sub_bit < 8 && in.ok; ++sub_bit) {
                        if (sub & (1 << sub_bit)) {
                            keys.push_back(base + (group * 8 + static_cast<uint32_t>(bit)) * 8 + static_cast<uint32_t>(sub_bit));
                        }
                    }
                }
            }
        }
    } else {
        log_line("data blob key format %d rejected", key_fmt);
        return false;
    }
    if (!in.ok) {
        return false;
    }
    double shared = 0;
    bool use_shared = value_fmt == 3;
    if (use_shared) {
        shared = in.f32();
    }
    for (uint32_t key : keys) {
        DataPair pair;
        pair.key = key;
        double wide = shared;
        if (value_fmt == 1) {
            wide = in.f32();
        } else if (value_fmt == 2) {
            wide = static_cast<double>(static_cast<int16_t>(in.u16())) * 0.01;
        } else if (value_fmt == 4) {
            wide = in.be7();
        } else if (value_fmt == 5) {
            in.take(&pair.bits, 8);
            if (!in.ok) {
                return false;
            }
            out.push_back(pair);
            continue;
        } else if (!use_shared) {
            log_line("data blob value format %d rejected", value_fmt);
            return false;
        }
        if (!in.ok) {
            return false;
        }
        memcpy(&pair.bits, &wide, 8);
        out.push_back(pair);
    }
    return in.ok;
}

double pair_number(const DataPair& pair) {
    double value = 0;
    memcpy(&value, &pair.bits, 8);
    return value;
}

void split_affix_word(uint32_t key, uint64_t word) {
    uint32_t low = static_cast<uint32_t>(word & 0x3fffffu);
    uint32_t mid = static_cast<uint32_t>((word >> 22) & 0x3fffffu);
    uint32_t high = static_cast<uint32_t>(word >> 44);
    if (low) {
        set_num(16, static_cast<int>(key), low);
    }
    if (mid) {
        set_num(17, static_cast<int>(key), mid);
    }
    if (high) {
        set_num(20, static_cast<int>(key), high);
    }
}

void apply_affix_bit(uint32_t key, double value) {
    int bit = static_cast<int>(value + 0.01) - 1;
    if (bit < 0 || bit > 63) {
        return;
    }
    int cat = 16;
    int shift = bit;
    if (bit >= 44) {
        cat = 20;
        shift = bit - 44;
    } else if (bit >= 22) {
        cat = 17;
        shift = bit - 22;
    }
    uint32_t mask = trunc_num(game_num(cat, static_cast<int>(key)) + 0.01);
    mask |= 1u << shift;
    set_num(cat, static_cast<int>(key), mask);
}

bool apply_named_blob(PktReader& in, bool bits) {
    uint32_t nbytes = in.u32();
    if (!in.ok || nbytes > 0xffffff || in.off + nbytes > in.size) {
        in.ok = false;
        return false;
    }
    if (nbytes == 0) {
        return true;
    }
    PktReader blob{in.data + in.off, nbytes};
    in.off += nbytes;
    std::vector<DataPair> pairs;
    if (!decode_pairs(blob, pairs)) {
        return false;
    }
    for (const DataPair& pair : pairs) {
        if (bits) {
            apply_affix_bit(pair.key, pair_number(pair));
        } else {
            split_affix_word(pair.key, pair.bits);
        }
    }
    return true;
}

void apply_affix_snapshot(PktReader& in) {
    clear_cat(16);
    clear_cat(17);
    clear_cat(20);
    if (!apply_named_blob(in, false) || !apply_named_blob(in, true)) {
        log_line("affix snapshot rejected");
        in.ok = false;
        return;
    }
    log_line("affix snapshot cats 16=%zu 17=%zu 20=%zu",
             g_game[16].nums.size(), g_game[17].nums.size(), g_game[20].nums.size());
}

void apply_item_block(PktReader& in) {
    uint16_t count = in.u16();
    uint32_t key = 0;
    for (uint16_t i = 0; i < count && in.ok; ++i) {
        key += in.be7();
        uint8_t flags = in.u8();
        int f04 = -1;
        int f08 = flags & 1;
        uint32_t f0c = 0;
        uint32_t f10 = 0;
        int f18 = 0;
        uint32_t f14 = 0;
        bool has18 = false;
        if (flags & 2) {
            f04 = in.u8();
        } else if (flags & 4) {
            f04 = static_cast<int>(in.be7());
        }
        if (flags & 8) {
            f0c = in.be7();
        }
        if (flags & 0x10) {
            f10 = in.be7();
        }
        if (flags & 0x20) {
            f18 = in.u8();
            has18 = true;
        } else if (flags & 0x40) {
            f18 = static_cast<int>(in.u32());
            has18 = true;
        }
        if (flags & 0x80) {
            f14 = in.be7();
        }
        if (!in.ok) {
            return;
        }
        if (f04 >= 0) {
            set_num(1, static_cast<int>(key), f04 + 1);
        }
        if (f08) {
            set_num(3, static_cast<int>(key), 1);
        }
        if (f14) {
            set_num(10, static_cast<int>(key), f14);
        }
        if (f0c) {
            set_num(6, static_cast<int>(key), f0c);
        }
        if (f0c && f10) {
            set_num(7, static_cast<int>(f0c), f10);
        }
        if (f0c && has18) {
            set_num(4, static_cast<int>(f0c), f18 >= 0 ? f18 : -f18 / 1000.0);
        }
    }
    uint8_t listed = in.u8();
    for (uint8_t i = 0; i < listed && in.ok; ++i) {
        set_num(2, i + 1, in.u8());
    }
    log_line("init-game perks learned=%zu active=%zu", g_game[1].nums.size(), g_game[3].nums.size());
}

void apply_attune_block(PktReader& in) {
    uint8_t groups = in.u8();
    int written = 0;
    log_line("init-game attune groups=%u at %u", groups, in.off);
    for (uint8_t g = 0; g < groups && in.ok; ++g) {
        uint8_t group = in.u8();
        std::vector<DataPair> pairs;
        if (!decode_pairs(in, pairs)) {
            in.ok = false;
            return;
        }
        for (const DataPair& pair : pairs) {
            set_num(11, static_cast<int>(pair.key | (static_cast<uint32_t>(group) << 24)), pair_number(pair));
            ++written;
        }
    }
    uint16_t count = in.u16();
    uint32_t key = 0;
    for (uint16_t i = 0; i < count && in.ok; ++i) {
        key += in.be7();
        int raw = static_cast<int16_t>(in.u16());
        set_num(11, static_cast<int>(key), raw * 0.01);
        ++written;
    }
    log_line("init-game attune values=%d cat11=%zu", written, g_game[11].nums.size());
}

void apply_init_game(const uint8_t* rest, uint32_t size) {
    PktReader in{rest, size};
    uint8_t compressed = in.u8();
    uint32_t comp = in.u32();
    uint32_t raw = in.u32();
    if (!in.ok || comp == 0 || comp > 0x10000 || raw == 0 || raw > 0x20000 || in.off + comp > in.size) {
        log_line("init-game rejected comp=%u raw=%u size=%u", comp, raw, size);
        return;
    }
    std::vector<char> plain(raw);
    const char* body = reinterpret_cast<const char*>(in.data + in.off);
    int body_size = static_cast<int>(comp);
    if (compressed) {
        int got = LZ4_decompress_safe(body, plain.data(), body_size, static_cast<int>(raw));
        if (got != static_cast<int>(raw)) {
            log_line("init-game decompress %d wanted %u", got, raw);
            return;
        }
        body = plain.data();
        body_size = got;
    }
    PktReader game{reinterpret_cast<const uint8_t*>(body), static_cast<uint32_t>(body_size)};
    uint8_t has_items = game.u8();
    if (has_items) {
        apply_item_block(game);
    }
    uint8_t flags = game.u8();
    if (!game.ok) {
        log_line("init-game header short");
        return;
    }
    if ((flags & 2) == 0) {
        clear_cat(11);
    }
    if (flags & 1) {
        apply_attune_block(game);
    }
    log_line("init-game flags=%u items=%u ok=%d", flags, has_items, game.ok ? 1 : 0);
}

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
            apply_affix_snapshot(in);
            if (!in.ok) {
                break;
            }
            ++applied;
            continue;
        }
        if (kind <= 2) {
            int cat = static_cast<int>(in.u32());
            uint32_t bytes = in.u32();
            if (!in.ok || in.off + bytes > in.size) {
                in.ok = false;
                break;
            }
            PktReader sub{in.data + in.off, bytes};
            in.off += bytes;
            std::vector<DataPair> pairs;
            if (!decode_pairs(sub, pairs)) {
                log_line("game data cat %d kind %u rejected", cat, kind);
                continue;
            }
            if (kind == 1) {
                clear_nums(cat);
            }
            for (const DataPair& pair : pairs) {
                set_num(cat, static_cast<int>(pair.key), pair_number(pair));
            }
            log_line("game data cat %d kind %u pairs %zu", cat, kind, pairs.size());
            ++applied;
            continue;
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

void apply_loot_filter(const uint8_t* rest, uint32_t size) {
    if (!rest || size < 5 || rest[0] != 1) {
        return;
    }
    uint32_t nbytes = 0;
    memcpy(&nbytes, rest + 1, 4);
    if (nbytes == 0 || nbytes > 0xffff || 5u + nbytes > size) {
        log_line("loot filter rejected len=%u size=%u", nbytes, size);
        return;
    }
    loot_filter_obj local{};
    for (int i = 0; i < 32; ++i) {
        loot_import_rule_init(local.rules[i]);
    }
    loot_import_buf in{const_cast<uint8_t*>(rest + 5), 0, static_cast<int32_t>(nbytes), 0};
    bool ok = loot_import_parse(&local, &in);
    if (ok) {
        if (!loot_filter) {
            loot_filter = new loot_filter_obj{};
            for (int i = 0; i < 32; ++i) {
                loot_import_rule_init(loot_filter->rules[i]);
            }
        }
        loot_import_commit(loot_filter, &local);
        log_line("loot filter rules=%u options=%u", loot_filter->rule_count, loot_filter->options);
    } else {
        log_line("loot filter import failed at %d/%u", in.pos, nbytes);
    }
    for (int i = 0; i < 32; ++i) {
        loot_import_rule_free(local.rules[i]);
    }
}

// ʕ •ᴥ•ʔ✿ Bit 15 of the slot word is the name flag. Slots 0..14 are item ids at set*100+11. ✿ ʕ •ᴥ•ʔ
void apply_transmog_sets(const uint8_t* rest, uint32_t size) {
    PktReader in{rest, size};
    if (in.u8() != 0) {
        return;
    }
    int count = static_cast<int8_t>(in.u8());
    int active = static_cast<int8_t>(in.u8());
    uint16_t options = in.u16();
    if (!in.ok) {
        return;
    }
    set_num(26, 1, active);
    set_num(26, 2, options);
    int items = 0;
    int names = 0;
    for (int set = 1; set <= count && in.ok; ++set) {
        int base = set * 100;
        uint16_t bits = in.u16();
        for (int slot = 0; slot < 15 && in.ok; ++slot) {
            if ((bits & (1u << slot)) == 0) {
                continue;
            }
            set_num(26, base + 11 + slot, in.u16());
            ++items;
        }
        if (in.ok && static_cast<int16_t>(bits) < 0) {
            uint8_t len = in.u8();
            if (!in.ok || in.off + len > in.size) {
                in.ok = false;
            } else {
                g_game[27].strs[set].assign(reinterpret_cast<const char*>(in.data + in.off), len);
                in.off += len;
                ++names;
            }
        }
        uint8_t enchants = in.u8();
        for (int slot = 0; slot < 4 && in.ok; ++slot) {
            if ((enchants & (1u << slot)) == 0) {
                continue;
            }
            set_num(26, base + 3 + slot, in.u16());
        }
        if (in.ok) {
            set_num(26, base + 1, in.u16());
        }
    }
    log_line("transmog sets=%d active=%d items=%d names=%d ok=%d", count, active, items, names, in.ok ? 1 : 0);
}

void syn_features_packet(int cmd, const uint8_t* rest, uint32_t size) {
    if (cmd == 7 && rest) {
        apply_init_game(rest, size);
    }
    if (cmd == 13 && rest) {
        apply_game_data(rest, size);
    }
    if (cmd == 14 && rest) {
        adopt_classes(rest, size);
    }
    if (cmd == 15 && rest) {
        apply_loot_filter(rest, size);
    }
    if (cmd == 17 && rest) {
        apply_transmog_sets(rest, size);
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

constexpr uint32_t kTradeSkillToolsRecSize = 0x2b8;
constexpr int kTradeSkillToolsFocus = 0x48;
constexpr int kTradeSkillToolsTotem = 0xc8;
// ʕ •ᴥ•ʔ✿ four 8-byte strings at 0x220 shift the 32-bit tail by 0x10 ✿ ʕ •ᴥ•ʔ
constexpr int kTradeSkillToolsTotemCat = 0x288;
constexpr int kTradeSkillToolsArea = 0x290;
constexpr int kTradeSkillToolsName = 8;
constexpr int kTradeSkillToolsAreaName = 0x30;
constexpr int kTradeSkillToolsGroupArea = 4;
constexpr uint32_t kTradeSkillToolsCatFlags = 0xffffffebu;

using TradeSkillToolsDecompressFn = uint8_t* (*)(const uint8_t*, uint32_t, uint8_t*);
using TradeSkillToolsActiveFn = uint64_t (*)();
using TradeSkillToolsObjectFn = void* (*)(uint64_t guid, uint32_t mask, const char* file, int line);
using TradeSkillToolsBagFn = void* (*)(void* player);
using TradeSkillToolsFindItemFn = void* (*)(void* bag, int32_t item, uint32_t flags);
using TradeSkillToolsFindCatFn = bool (*)(void* bag, int32_t category, uint32_t flags);

struct TradeSkillToolsBind {
    const uint8_t* spells = nullptr;
    const int32_t* compress = nullptr;
    TradeSkillToolsDecompressFn decompress = nullptr;
    const uint8_t* focus = nullptr;
    const uint8_t* groups = nullptr;
    const uint8_t* areas = nullptr;
    const uint8_t* categories = nullptr;
    TradeSkillToolsActiveFn active = nullptr;
    TradeSkillToolsObjectFn object = nullptr;
    TradeSkillToolsBagFn bag = nullptr;
    TradeSkillToolsFindItemFn find_item = nullptr;
    TradeSkillToolsFindCatFn find_cat = nullptr;
};

TradeSkillToolsBind& tradeskill_tools_bind() {
    static TradeSkillToolsBind bind;
    static bool ready = false;
    if (!ready) {
        ready = true;
        void* found = nullptr;
        if (syn_resolve("g_spellDB", &found)) {
            bind.spells = static_cast<const uint8_t*>(found);
        }
        found = nullptr;
        if (syn_resolve("g_clientDbPerformCompress", &found)) {
            bind.compress = static_cast<const int32_t*>(found);
        }
        found = nullptr;
        if (syn_resolve("_Z13DecompressRowPKhjPh", &found)) {
            bind.decompress = reinterpret_cast<TradeSkillToolsDecompressFn>(found);
        }
        found = nullptr;
        if (syn_resolve("g_spellFocusObjectDB", &found)) {
            bind.focus = static_cast<const uint8_t*>(found);
        }
        found = nullptr;
        if (syn_resolve("g_areaGroupDB", &found)) {
            bind.groups = static_cast<const uint8_t*>(found);
        }
        found = nullptr;
        if (syn_resolve("g_areaTableDB", &found)) {
            bind.areas = static_cast<const uint8_t*>(found);
        }
        found = nullptr;
        if (syn_resolve("g_totemCategoryDB", &found)) {
            bind.categories = static_cast<const uint8_t*>(found);
        }
        found = nullptr;
        if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
            bind.active = reinterpret_cast<TradeSkillToolsActiveFn>(found);
        }
        found = nullptr;
        if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
            bind.object = reinterpret_cast<TradeSkillToolsObjectFn>(found);
        }
        found = nullptr;
        if (syn_resolve("_ZNK10CGPlayer_C6GetBagEv", &found)) {
            bind.bag = reinterpret_cast<TradeSkillToolsBagFn>(found);
        }
        found = nullptr;
        if (syn_resolve("_ZN7CGBag_C14FindItemOfTypeEij", &found)) {
            bind.find_item = reinterpret_cast<TradeSkillToolsFindItemFn>(found);
        }
        found = nullptr;
        if (syn_resolve("_ZN7CGBag_C17FindTotemCategoryEij", &found)) {
            bind.find_cat = reinterpret_cast<TradeSkillToolsFindCatFn>(found);
        }
    }
    return bind;
}

bool tradeskill_tools_id(double value, int* out) {
    if (!(value > -2147483648.0)) {
        return false;
    }
    if (!(value < 2147483647.0)) {
        *out = 0x7fffffff;
        return true;
    }
    if (!(value >= 1.0)) {
        return false;
    }
    int id = static_cast<int>(value);
    if (id <= 0) {
        return false;
    }
    *out = id;
    return true;
}

const uint8_t* tradeskill_tools_spell(int id, uint8_t* storage) {
    const TradeSkillToolsBind& bind = tradeskill_tools_bind();
    if (!bind.spells) {
        return nullptr;
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&max_id, bind.spells + 0x10, 4);
    memcpy(&min_id, bind.spells + 0x14, 4);
    if (id < min_id || id > max_id) {
        return nullptr;
    }
    const uint8_t* const* by_id = nullptr;
    memcpy(&by_id, bind.spells + 0x30, sizeof(by_id));
    if (!by_id) {
        return nullptr;
    }
    const uint8_t* raw = by_id[id - min_id];
    if (!raw) {
        return nullptr;
    }
    if (bind.compress && *bind.compress) {
        if (!bind.decompress) {
            return nullptr;
        }
        bind.decompress(raw, kTradeSkillToolsRecSize, storage);
        return storage;
    }
    return raw;
}

const uint8_t* tradeskill_tools_record(const uint8_t* table, int id) {
    if (!table) {
        return nullptr;
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&max_id, table + 0x10, 4);
    memcpy(&min_id, table + 0x14, 4);
    if (id < min_id || id > max_id) {
        return nullptr;
    }
    const uint8_t* const* by_id = nullptr;
    memcpy(&by_id, table + 0x30, sizeof(by_id));
    if (!by_id) {
        return nullptr;
    }
    return by_id[id - min_id];
}

int32_t tradeskill_tools_i32(const uint8_t* row, int offset) {
    int32_t value = 0;
    memcpy(&value, row + offset, 4);
    return value;
}

const char* tradeskill_tools_field(const uint8_t* row, int offset) {
    const char* text = nullptr;
    memcpy(&text, row + offset, sizeof(text));
    return text;
}

void* tradeskill_tools_player() {
    const TradeSkillToolsBind& bind = tradeskill_tools_bind();
    if (!bind.active || !bind.object) {
        return nullptr;
    }
    return bind.object(bind.active(), 0x10, "test:1", 0xa0);
}

void tradeskill_tools_pair(void* state, const char* text, bool owned, int* count) {
    if (text) {
        g_push_str(state, text);
    } else {
        g_push_nil(state);
    }
    if (owned) {
        g_push_num(state, 1.0);
    } else {
        g_push_nil(state);
    }
    *count += 1;
}

// ʕ •ᴥ•ʔ✿ spell id; each present tool is name, then 1 or nil ✿ ʕ •ᴥ•ʔ
int native_get_tradeskill_tools(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_push_str || !g_push_nil || !g_is_num(state, 1)) {
        return 0;
    }
    int id = 0;
    if (!tradeskill_tools_id(g_to_num(state, 1), &id)) {
        return 0;
    }
    void* player = tradeskill_tools_player();
    if (!player) {
        return 0;
    }
    alignas(8) uint8_t storage[kTradeSkillToolsRecSize];
    const uint8_t* row = tradeskill_tools_spell(id, storage);
    if (!row) {
        return 0;
    }
    const TradeSkillToolsBind& bind = tradeskill_tools_bind();
    void* bag = bind.bag ? bind.bag(player) : nullptr;
    int count = 0;
    int32_t focus = tradeskill_tools_i32(row, kTradeSkillToolsFocus);
    if (focus) {
        const uint8_t* rec = tradeskill_tools_record(bind.focus, focus);
        if (rec) {
            tradeskill_tools_pair(state, tradeskill_tools_field(rec, kTradeSkillToolsName), true, &count);
        }
    }
    int32_t group_id = tradeskill_tools_i32(row, kTradeSkillToolsArea);
    if (group_id > 0) {
        const uint8_t* group = tradeskill_tools_record(bind.groups, group_id);
        if (group) {
            const uint8_t* area = tradeskill_tools_record(bind.areas, tradeskill_tools_i32(group, kTradeSkillToolsGroupArea));
            if (area) {
                tradeskill_tools_pair(state, tradeskill_tools_field(area, kTradeSkillToolsAreaName), true, &count);
            }
        }
    }
    for (int i = 0; i < 2; ++i) {
        int32_t totem = tradeskill_tools_i32(row, kTradeSkillToolsTotem + i * 4);
        if (totem <= 0 || static_cast<uint32_t>(totem) > 0xffffu) {
            continue;
        }
        const ItemRow* item = item_row(totem);
        if (!item) {
            continue;
        }
        bool owned = bag && bind.find_item && bind.find_item(bag, totem, 0);
        tradeskill_tools_pair(state, item->name.c_str(), owned, &count);
    }
    for (int i = 0; i < 2; ++i) {
        int32_t category = tradeskill_tools_i32(row, kTradeSkillToolsTotemCat + i * 4);
        if (!category) {
            continue;
        }
        const uint8_t* rec = tradeskill_tools_record(bind.categories, category);
        if (!rec) {
            continue;
        }
        bool owned = bag && bind.find_cat && bind.find_cat(bag, category, kTradeSkillToolsCatFlags);
        tradeskill_tools_pair(state, tradeskill_tools_field(rec, kTradeSkillToolsName), owned, &count);
    }
    return count * 2;
}

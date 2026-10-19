constexpr int kRecipeSpellLimit = 0x1fbd0;
constexpr uint32_t kRecipeSpellSize = 0x2b8;
constexpr int kRecipeAttr = 0x10;
constexpr int kRecipeReagent = 0xd0;
constexpr int kRecipeReagentCount = 0xf0;
constexpr int kRecipeEquipClass = 0x110;
constexpr int kRecipeEquipSub = 0x114;
constexpr int kRecipeEquipInv = 0x118;
constexpr int kRecipeEffect = 0x11c;
constexpr int kRecipeBase = 0x140;
constexpr int kRecipeItemType = 0x1ac;
constexpr int kRecipeName = 0x220;
constexpr int kRecipeSkillVerb = 0x28;
constexpr int kRecipeSubDisplay = 0x28;
constexpr int kRecipeSubVerbose = 0x30;
constexpr int kRecipeSubStride = 0x40;
constexpr uint32_t kRecipeTradeBit = 0x20;

using RecipeDecompressFn = uint8_t* (*)(const uint8_t*, uint32_t, uint8_t*);
using RecipeActiveFn = uint64_t (*)();
using RecipeObjectFn = void* (*)(uint64_t, uint32_t, const char*, int);
using RecipeAbilityFn = void* (*)(void*, int);
using RecipeSkillFn = int32_t (*)(const void*, int32_t);
using RecipeBagFn = void* (*)(void*);
using RecipeCountFn = int32_t (*)(void*, int32_t, uint32_t);

struct RecipeBind {
    const uint8_t* spells = nullptr;
    const int32_t* compress = nullptr;
    RecipeDecompressFn decompress = nullptr;
    const uint8_t* skills = nullptr;
    const uint8_t* subclasses = nullptr;
    RecipeActiveFn active = nullptr;
    RecipeObjectFn object = nullptr;
    RecipeAbilityFn ability = nullptr;
    RecipeSkillFn skill_index = nullptr;
    RecipeBagFn bag = nullptr;
    RecipeCountFn count = nullptr;
};

RecipeBind& recipe_bind() {
    static RecipeBind bind;
    static bool ready = false;
    if (ready) {
        return bind;
    }
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
        bind.decompress = reinterpret_cast<RecipeDecompressFn>(found);
    }
    found = nullptr;
    if (syn_resolve("g_skillLineDB", &found)) {
        bind.skills = static_cast<const uint8_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("g_itemSubClassDB", &found)) {
        bind.subclasses = static_cast<const uint8_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
        bind.active = reinterpret_cast<RecipeActiveFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
        bind.object = reinterpret_cast<RecipeObjectFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN10CGUnit_C13LookupAbilityEi", &found)) {
        bind.ability = reinterpret_cast<RecipeAbilityFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZNK10CGPlayer_C13GetSkillIndexEi", &found)) {
        bind.skill_index = reinterpret_cast<RecipeSkillFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZNK10CGPlayer_C6GetBagEv", &found)) {
        bind.bag = reinterpret_cast<RecipeBagFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN7CGBag_C16GetItemTypeCountEij", &found)) {
        bind.count = reinterpret_cast<RecipeCountFn>(found);
    }
    return bind;
}

double recipe_floor(double n) {
    double whole = static_cast<double>(static_cast<int64_t>(n));
    if (n >= 0.0 || whole == n) {
        return whole;
    }
    return whole - 1.0;
}

bool recipe_spell_id(double value, int* out) {
    if (!(value > -2147483648.0) || !(value < 2147483647.0)) {
        return false;
    }
    double floored = recipe_floor(value);
    if (!(floored > -2147483648.0) || !(floored < 2147483647.0)) {
        return false;
    }
    int id = static_cast<int>(floored);
    if (id < 0 || id >= kRecipeSpellLimit) {
        return false;
    }
    *out = id;
    return true;
}

int32_t recipe_i32(const uint8_t* row, int offset) {
    int32_t value = 0;
    memcpy(&value, row + offset, 4);
    return value;
}

const char* recipe_ptr(const uint8_t* row, int offset) {
    const char* text = nullptr;
    memcpy(&text, row + offset, sizeof(text));
    return text;
}

const uint8_t* recipe_spell(int id, uint8_t* storage) {
    const RecipeBind& bind = recipe_bind();
    if (!bind.spells) {
        return nullptr;
    }
    int32_t max_id = recipe_i32(bind.spells, 0x10);
    int32_t min_id = recipe_i32(bind.spells, 0x14);
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
        bind.decompress(raw, kRecipeSpellSize, storage);
        return storage;
    }
    return raw;
}

void* recipe_player() {
    const RecipeBind& bind = recipe_bind();
    if (!bind.active || !bind.object) {
        return nullptr;
    }
    return bind.object(bind.active(), 0x10, "test:1", 0xa0);
}

const uint8_t* recipe_by_id(const uint8_t* table, int32_t id) {
    if (!table) {
        return nullptr;
    }
    int32_t max_id = recipe_i32(table, 0x10);
    int32_t min_id = recipe_i32(table, 0x14);
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

bool recipe_creates(int32_t effect) {
    return effect == 0x18 || effect == 0x9d || effect == 0x3b;
}

int32_t recipe_held(int item) {
    const RecipeBind& bind = recipe_bind();
    void* player = recipe_player();
    void* bag = player && bind.bag ? bind.bag(player) : nullptr;
    int32_t total = bag && bind.count ? bind.count(bag, item, 0) : 0;
    double extra = recipe_floor(game_num(13, item));
    if (extra > 0.0 && extra < 2147483647.0) {
        total += static_cast<int32_t>(extra);
    }
    return total;
}

int32_t recipe_available(const uint8_t* spell) {
    if (!spell) {
        return 0;
    }
    int32_t crafts[8] = {};
    for (int i = 0; i < 8; ++i) {
        int32_t item = recipe_i32(spell, kRecipeReagent + i * 4);
        uint32_t need = static_cast<uint32_t>(recipe_i32(spell, kRecipeReagentCount + i * 4));
        if (!item || need == 0) {
            continue;
        }
        uint32_t made = static_cast<uint32_t>(recipe_held(item)) / need;
        if (static_cast<int32_t>(made) <= 0) {
            return 0;
        }
        crafts[i] = static_cast<int32_t>(made);
    }
    int32_t best = 0x7fffffff;
    for (int i = 0; i < 8; ++i) {
        if (crafts[i] > 0 && crafts[i] < best) {
            best = crafts[i];
        }
    }
    return best == 0x7fffffff ? 0 : best;
}

const char* recipe_subclass(int klass, int sub) {
    const uint8_t* table = recipe_bind().subclasses;
    if (!table) {
        return nullptr;
    }
    int32_t count = recipe_i32(table, 0x0c);
    const uint8_t* rows = nullptr;
    memcpy(&rows, table + 0x28, sizeof(rows));
    if (!rows || count <= 0) {
        return nullptr;
    }
    for (int32_t i = 0; i < count; ++i) {
        const uint8_t* rec = rows + static_cast<size_t>(i) * kRecipeSubStride;
        if (recipe_i32(rec, 0) != klass || recipe_i32(rec, 4) != sub) {
            continue;
        }
        const char* verbose = recipe_ptr(rec, kRecipeSubVerbose);
        if (verbose && verbose[0]) {
            return verbose;
        }
        return recipe_ptr(rec, kRecipeSubDisplay);
    }
    return nullptr;
}

uint32_t recipe_inv_mask(uint32_t inv) {
    // ʕ •ᴥ•ʔ✿ inventory-type bit to the tradeskill slot mask, first set bit only ✿ ʕ •ᴥ•ʔ
    static const uint32_t kSlots[29] = {
        0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 3072, 12288, 98304, 65536, 131072,
        16384, 98304, 7864320, 262144, 16, 98304, 98304, 65536, 0, 131072, 131072, 0, 131072,
    };
    uint32_t bit = 1;
    for (int i = 0; i < 29; ++i) {
        if (inv & bit) {
            uint32_t mask = kSlots[i];
            if (mask & 0x80000) {
                mask &= 0xff8fffff;
            } else if (mask & 0x400) {
                mask &= ~0x800u;
            } else if (mask & 0x1000) {
                mask &= ~0x2000u;
            }
            return mask ? mask : 0x4000000u;
        }
        bit <<= 1;
    }
    return 0x4000000u;
}

const char* recipe_slot_token(const uint8_t* spell, int32_t item) {
    if (item > 0 && static_cast<uint32_t>(item) <= 0xffffu) {
        const ItemRow* row = item_row(item);
        if (row) {
            return recipe_subclass(static_cast<int8_t>(row->klass), static_cast<int8_t>(row->sub));
        }
    }
    int32_t klass = recipe_i32(spell, kRecipeEquipClass);
    int32_t sub = recipe_i32(spell, kRecipeEquipSub);
    if (klass == 2) {
        return (sub & 1) ? "0x800000" : "0x1000000";
    }
    if (klass == 4) {
        uint32_t inv = static_cast<uint32_t>(recipe_i32(spell, kRecipeEquipInv));
        if (!inv) {
            return (sub & 0x40) ? "0x2000000" : "0x4000000";
        }
        return nullptr;
    }
    return (sub & 0x40) ? "0x2000000" : "0x4000000";
}

void recipe_push_slot(void* state, const uint8_t* spell, int32_t item) {
    int32_t klass = recipe_i32(spell, kRecipeEquipClass);
    uint32_t inv = static_cast<uint32_t>(recipe_i32(spell, kRecipeEquipInv));
    bool format = klass == 4 && inv && !(item > 0 && static_cast<uint32_t>(item) <= 0xffffu && item_row(item));
    if (format) {
        char text[16];
        snprintf(text, sizeof(text), "0x%X", recipe_inv_mask(inv));
        g_push_str(state, text);
        return;
    }
    const char* token = recipe_slot_token(spell, item);
    if (token) {
        g_push_str(state, token);
    } else {
        g_push_nil(state);
    }
}

const char* recipe_difficulty(const uint8_t* ability) {
    static const char* kWords[] = {"optimal", "medium", "easy", "trivial"};
    const RecipeBind& bind = recipe_bind();
    void* player = recipe_player();
    if (!player || !bind.skill_index || !ability) {
        return nullptr;
    }
    int32_t high = recipe_i32(ability, 0x28);
    int32_t low = recipe_i32(ability, 0x2c);
    if (!low) {
        int32_t adjusted = high - 25;
        low = adjusted > 0 ? adjusted : 0;
    }
    int32_t index = bind.skill_index(player, recipe_i32(ability, 4));
    if (index < 0) {
        return nullptr;
    }
    const uint8_t* data = nullptr;
    memcpy(&data, static_cast<uint8_t*>(player) + 0x1890, sizeof(data));
    if (!data) {
        return nullptr;
    }
    uint16_t rank = 0;
    memcpy(&rank, data + 0x7a4 + static_cast<size_t>(index) * 12, 2);
    int32_t skill = rank;
    int word = 0;
    if (skill >= low) {
        int32_t mid = (low + high) / 2;
        if (skill < mid) {
            word = 1;
        } else if (skill >= high) {
            word = 3;
        } else {
            word = 2;
        }
    }
    return kWords[word];
}

void recipe_push_text(void* state, const char* text) {
    if (text) {
        g_push_str(state, text);
    } else {
        g_push_nil(state);
    }
}

// ʕ •ᴥ•ʔ✿ spell id; skill line, name, created item, count made, num available, alt verb, slot token, difficulty ✿ ʕ •ᴥ•ʔ
int native_get_profession_recipe_info(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_push_str || !g_push_nil || !g_is_num(state, 1)) {
        return 0;
    }
    int id = 0;
    if (!recipe_spell_id(g_to_num(state, 1), &id)) {
        return 0;
    }
    alignas(8) uint8_t storage[kRecipeSpellSize];
    const uint8_t* spell = recipe_spell(id, storage);
    if (!spell || (static_cast<uint8_t>(recipe_i32(spell, kRecipeAttr)) & kRecipeTradeBit) == 0) {
        return 0;
    }
    void* player = recipe_player();
    RecipeAbilityFn lookup = recipe_bind().ability;
    const uint8_t* ability = player && lookup ? static_cast<const uint8_t*>(lookup(player, recipe_i32(spell, 0))) : nullptr;
    if (!ability) {
        return 0;
    }
    g_push_num(state, recipe_i32(ability, 4));
    const char* name = recipe_ptr(spell, kRecipeName);
    g_push_str(state, name ? name : "");
    int32_t effect = recipe_i32(spell, kRecipeEffect);
    int32_t created = recipe_creates(effect) ? recipe_i32(spell, kRecipeItemType) : 0;
    g_push_num(state, created);
    g_push_num(state, static_cast<int32_t>(static_cast<uint32_t>(recipe_i32(spell, kRecipeBase)) + 1u));
    g_push_num(state, recipe_available(spell));
    if (recipe_creates(effect)) {
        g_push_nil(state);
    } else {
        const uint8_t* line = recipe_by_id(recipe_bind().skills, recipe_i32(ability, 4));
        recipe_push_text(state, line ? recipe_ptr(line, kRecipeSkillVerb) : nullptr);
    }
    recipe_push_slot(state, spell, created);
    recipe_push_text(state, recipe_difficulty(ability));
    return 8;
}

constexpr uint32_t do_recipe_id_max = 0x1fbcf;
constexpr uint32_t do_recipe_row_bytes = 0x2b8;
constexpr int do_recipe_attributes = 0x10;
constexpr uint32_t do_recipe_tradespell = 0x20;
constexpr double do_recipe_int_min = -2147483648.0;
constexpr double do_recipe_int_max = 2147483647.0;

using DoRecipeDecompress = uint8_t* (*)(const uint8_t*, uint32_t, uint8_t*);
using DoRecipeHas = bool (*)(uint32_t);
using DoRecipeCast = bool (*)(int, const void*, uint64_t, bool);
using DoRecipeTrade = void (*)(int, uint32_t);

struct DoRecipeFns {
    const uint8_t* spells = nullptr;
    const int32_t* compress = nullptr;
    DoRecipeDecompress decompress = nullptr;
    DoRecipeHas has = nullptr;
    DoRecipeCast cast = nullptr;
    DoRecipeTrade trade = nullptr;
    int32_t* recast = nullptr;
};

DoRecipeFns& do_recipe_fns() {
    static DoRecipeFns do_recipe_state;
    static bool do_recipe_ready = false;
    if (do_recipe_ready) {
        return do_recipe_state;
    }
    do_recipe_ready = true;
    void* found = nullptr;
    if (syn_resolve("g_spellDB", &found)) {
        do_recipe_state.spells = static_cast<const uint8_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("g_clientDbPerformCompress", &found)) {
        do_recipe_state.compress = static_cast<const int32_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("_Z13DecompressRowPKhjPh", &found)) {
        do_recipe_state.decompress = reinterpret_cast<DoRecipeDecompress>(found);
    }
    found = nullptr;
    if (syn_resolve("_Z8HasSpellj", &found)) {
        do_recipe_state.has = reinterpret_cast<DoRecipeHas>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN7Spell_C9CastSpellEiPK8CGItem_C7WOWGUIDb", &found)) {
        do_recipe_state.cast = reinterpret_cast<DoRecipeCast>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN16CGTradeSkillInfo12DoTradeSkillEij", &found)) {
        do_recipe_state.trade = reinterpret_cast<DoRecipeTrade>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN16CGTradeSkillInfo15m_recastSpellIDE", &found)) {
        do_recipe_state.recast = static_cast<int32_t*>(found);
    }
    return do_recipe_state;
}

// ʕ •ᴥ•ʔ✿ CRT round, half away from zero, then the DLL's range check ✿ ʕ •ᴥ•ʔ
double do_recipe_nearest(double value) {
    return value >= 0.0 ? value + 0.5 : value - 0.5;
}

bool do_recipe_spell_id(double value, int* out) {
    if (!(value > do_recipe_int_min) || !(value < do_recipe_int_max)) {
        return false;
    }
    double rounded = do_recipe_nearest(value);
    if (!(rounded > do_recipe_int_min) || !(rounded < do_recipe_int_max)) {
        return false;
    }
    int id = static_cast<int>(rounded);
    if (static_cast<uint32_t>(id) > do_recipe_id_max) {
        return false;
    }
    *out = id;
    return true;
}

bool do_recipe_times(double value, int* out) {
    if (!(value > do_recipe_int_min)) {
        return false;
    }
    if (!(value < do_recipe_int_max)) {
        *out = 0x7fffffff;
        return true;
    }
    double rounded = do_recipe_nearest(value);
    if (!(rounded > do_recipe_int_min)) {
        return false;
    }
    if (!(rounded < do_recipe_int_max)) {
        *out = 0x7fffffff;
        return true;
    }
    int times = static_cast<int>(rounded);
    if (times < 1) {
        return false;
    }
    *out = times;
    return true;
}

bool do_recipe_is_tradespell(int id) {
    const DoRecipeFns& fns = do_recipe_fns();
    if (!fns.spells) {
        return false;
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&max_id, fns.spells + 0x10, 4);
    memcpy(&min_id, fns.spells + 0x14, 4);
    if (id < min_id || id > max_id) {
        return false;
    }
    const uint8_t* const* by_id = nullptr;
    memcpy(&by_id, fns.spells + 0x30, sizeof(by_id));
    if (!by_id) {
        return false;
    }
    const uint8_t* raw = by_id[id - min_id];
    if (!raw) {
        return false;
    }
    const uint8_t* row = raw;
    alignas(8) uint8_t storage[do_recipe_row_bytes];
    if (fns.compress && *fns.compress) {
        if (!fns.decompress) {
            return false;
        }
        fns.decompress(raw, do_recipe_row_bytes, storage);
        row = storage;
    }
    int32_t attributes = 0;
    memcpy(&attributes, row + do_recipe_attributes, 4);
    return (static_cast<uint32_t>(attributes) & do_recipe_tradespell) != 0;
}

// ʕ •ᴥ•ʔ✿ spell id, optional repeat. Known tradeskill spell: cast unless that recast is current, then DoTradeSkill. Pushes 1. ✿ ʕ •ᴥ•ʔ
int native_do_profession_recipe(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int spell = 0;
    if (!do_recipe_spell_id(g_to_num(state, 1), &spell) || !do_recipe_is_tradespell(spell)) {
        return 0;
    }
    const DoRecipeFns& fns = do_recipe_fns();
    if (!fns.has || !fns.has(static_cast<uint32_t>(spell))) {
        return 0;
    }
    int times = 1;
    if (g_is_num(state, 2) && !do_recipe_times(g_to_num(state, 2), &times)) {
        return 0;
    }
    if (!fns.trade) {
        return 0;
    }
    if (!fns.recast || *fns.recast != spell) {
        if (!fns.cast || !fns.cast(spell, nullptr, 0, false)) {
            return 0;
        }
    }
    fns.trade(spell, static_cast<uint32_t>(times));
    g_push_num(state, 1.0);
    return 1;
}

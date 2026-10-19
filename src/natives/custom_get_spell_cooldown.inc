using SpellCooldownFn = int (*)(int, int, uint32_t*, uint32_t*, uint32_t*);
using AsyncMsFn = uint32_t (*)();

struct SpellCooldownBind {
    SpellCooldownFn cooldown = nullptr;
    AsyncMsFn now = nullptr;
};

SpellCooldownBind& spell_cooldown_bind() {
    static SpellCooldownBind bind;
    static bool ready = false;
    if (!ready) {
        ready = true;
        void* found = nullptr;
        if (syn_resolve("_Z24Spell_C_GetSpellCooldowniiPjS_S_", &found)) {
            bind.cooldown = reinterpret_cast<SpellCooldownFn>(found);
        }
        found = nullptr;
        if (syn_resolve("_Z16OsGetAsyncTimeMsv", &found)) {
            bind.now = reinterpret_cast<AsyncMsFn>(found);
        }
    }
    return bind;
}

bool spell_cooldown_id(double value, int* out) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return false;
    }
    if (!(value < k_max)) {
        *out = 0x7fffffff;
        return true;
    }
    double rounded = value >= 0.0
        ? static_cast<double>(static_cast<long long>(value + 0.5))
        : static_cast<double>(static_cast<long long>(value - 0.5));
    if (!(rounded > k_min)) {
        return false;
    }
    if (!(rounded < k_max)) {
        *out = 0x7fffffff;
        return true;
    }
    int id = static_cast<int>(rounded);
    if (id <= 0) {
        return false;
    }
    *out = id;
    return true;
}

bool spell_cooldown_left(int spell_id, int* left) {
    const SpellCooldownBind& bind = spell_cooldown_bind();
    if (!bind.cooldown || !bind.now) {
        return false;
    }
    uint32_t duration = 0;
    uint32_t start = 0;
    uint32_t enable = 0;
    *left = 0;
    if (!bind.cooldown(spell_id, 0, &duration, &start, &enable) || !start || !duration) {
        return true;
    }
    uint32_t now = bind.now();
    uint32_t end = start + duration;
    if (static_cast<int32_t>(now) >= static_cast<int32_t>(end)) {
        return true;
    }
    *left = static_cast<int32_t>(end - now);
    return true;
}

// ʕ •ᴥ•ʔ✿ spell id; one number, remaining ms from the live cooldown, or nothing ✿ ʕ •ᴥ•ʔ
int native_get_spell_cooldown(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int spell_id = 0;
    int left = 0;
    if (!spell_cooldown_id(g_to_num(state, 1), &spell_id) || !spell_cooldown_left(spell_id, &left)) {
        return 0;
    }
    g_push_num(state, static_cast<double>(left));
    return 1;
}

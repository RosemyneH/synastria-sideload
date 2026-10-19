constexpr int kSpellEffectLimit = 0x1fbd0;
constexpr uint32_t kSpellEffectRecSize = 0x2b8;
constexpr int kSpellEffect = 0x11c;
constexpr int kSpellEffectDieSides = 0x128;
constexpr int kSpellEffectBase = 0x140;

using SpellEffectDecompressFn = uint8_t* (*)(const uint8_t*, uint32_t, uint8_t*);

struct SpellEffectBind {
    const uint8_t* db = nullptr;
    const int32_t* compress = nullptr;
    SpellEffectDecompressFn decompress = nullptr;
};

SpellEffectBind& spell_effect_bind() {
    static SpellEffectBind bind;
    static bool ready = false;
    if (!ready) {
        ready = true;
        void* found = nullptr;
        if (syn_resolve("g_spellDB", &found)) {
            bind.db = static_cast<const uint8_t*>(found);
        }
        found = nullptr;
        if (syn_resolve("g_clientDbPerformCompress", &found)) {
            bind.compress = static_cast<const int32_t*>(found);
        }
        found = nullptr;
        if (syn_resolve("_Z13DecompressRowPKhjPh", &found)) {
            bind.decompress = reinterpret_cast<SpellEffectDecompressFn>(found);
        }
    }
    return bind;
}

bool spell_effect_int(double value, int* out) {
    if (!(value > -2147483648.0) || !(value < 2147483647.0)) {
        return false;
    }
    double rounded = value >= 0.0
        ? static_cast<double>(static_cast<long long>(value + 0.5))
        : static_cast<double>(static_cast<long long>(value - 0.5));
    if (!(rounded > -2147483648.0) || !(rounded < 2147483647.0)) {
        return false;
    }
    *out = static_cast<int>(rounded);
    return true;
}

const uint8_t* spell_effect_row(int id, uint8_t* storage) {
    const SpellEffectBind& bind = spell_effect_bind();
    if (!bind.db) {
        return nullptr;
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&max_id, bind.db + 0x10, 4);
    memcpy(&min_id, bind.db + 0x14, 4);
    if (id < min_id || id > max_id) {
        return nullptr;
    }
    const uint8_t* const* by_id = nullptr;
    memcpy(&by_id, bind.db + 0x30, sizeof(by_id));
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
        bind.decompress(raw, kSpellEffectRecSize, storage);
        return storage;
    }
    return raw;
}

void spell_effect_push(void* state, const uint8_t* row, int offset) {
    uint32_t bits = 0;
    memcpy(&bits, row + offset, 4);
    g_push_num(state, static_cast<double>(bits));
}

// ʕ •ᴥ•ʔ✿ spell id, optional effect 0..2; effect, base points, die sides ✿ ʕ •ᴥ•ʔ
int native_get_spell_effect(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int id = 0;
    if (!spell_effect_int(g_to_num(state, 1), &id) || id <= 0 || id >= kSpellEffectLimit) {
        return 0;
    }
    int index = 0;
    if (g_is_num(state, 2)) {
        if (!spell_effect_int(g_to_num(state, 2), &index) || index < 0 || index >= 3) {
            return 0;
        }
    }
    alignas(8) uint8_t storage[kSpellEffectRecSize];
    const uint8_t* row = spell_effect_row(id, storage);
    if (!row) {
        return 0;
    }
    spell_effect_push(state, row, kSpellEffect + index * 4);
    spell_effect_push(state, row, kSpellEffectBase + index * 4);
    spell_effect_push(state, row, kSpellEffectDieSides + index * 4);
    return 3;
}

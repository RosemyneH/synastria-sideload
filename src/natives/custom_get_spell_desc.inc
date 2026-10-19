constexpr int kSpellLimit = 0x1fbd0;
constexpr uint32_t kSpellRecSize = 0x2b8;
constexpr int kSpellName = 0x220;
constexpr int kSpellSubtext = 0x228;
constexpr int kSpellDesc = 0x230;
constexpr int kSpellAura = 0x238;

using SpellParseFn = int (*)(const void*, char*, uint32_t, int, int, int, int, int, int);
using SpellDecompressFn = uint8_t* (*)(const uint8_t*, uint32_t, uint8_t*);

struct SpellDescBind {
    const uint8_t* db = nullptr;
    const int32_t* compress = nullptr;
    SpellDecompressFn decompress = nullptr;
    SpellParseFn parse = nullptr;
};

SpellDescBind& spell_desc_bind() {
    static SpellDescBind bind;
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
            bind.decompress = reinterpret_cast<SpellDecompressFn>(found);
        }
        found = nullptr;
        if (syn_resolve("_Z20SpellParserParseTextPK8SpellRecPcjiiiiii", &found)) {
            bind.parse = reinterpret_cast<SpellParseFn>(found);
        }
    }
    return bind;
}

bool spell_desc_int(double value, int* out) {
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

bool spell_desc_load(int id, uint8_t* storage) {
    const SpellDescBind& bind = spell_desc_bind();
    if (!bind.db) {
        return false;
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&max_id, bind.db + 0x10, 4);
    memcpy(&min_id, bind.db + 0x14, 4);
    if (id < min_id || id > max_id) {
        return false;
    }
    const uint8_t* const* by_id = nullptr;
    memcpy(&by_id, bind.db + 0x30, sizeof(by_id));
    if (!by_id) {
        return false;
    }
    const uint8_t* raw = by_id[id - min_id];
    if (!raw) {
        return false;
    }
    if (bind.compress && *bind.compress) {
        if (!bind.decompress) {
            return false;
        }
        bind.decompress(raw, kSpellRecSize, storage);
        return true;
    }
    memcpy(storage, raw, kSpellRecSize);
    return true;
}

const char* spell_desc_ptr(const uint8_t* row, int offset) {
    const char* text = nullptr;
    memcpy(&text, row + offset, sizeof(text));
    return text;
}

// ʕ •ᴥ•ʔ✿ id, optional field 0 name 1 rank 2 aura 3 description 4 parsed aura 5 parsed description ✿ ʕ •ᴥ•ʔ
int native_get_spell_desc(void* state) {
    if (!g_is_num || !g_to_num || !g_push_str || !g_is_num(state, 1)) {
        return 0;
    }
    int id = 0;
    if (!spell_desc_int(g_to_num(state, 1), &id) || id <= 0 || id >= kSpellLimit) {
        return 0;
    }
    int field = 0;
    if (g_is_num(state, 2) && !spell_desc_int(g_to_num(state, 2), &field)) {
        return 0;
    }
    if (static_cast<uint32_t>(field) > 5u) {
        return 0;
    }
    alignas(8) uint8_t storage[kSpellRecSize];
    if (!spell_desc_load(id, storage)) {
        return 0;
    }
    // ʕ •ᴥ•ʔ✿ four string pointers start at 0x220; each is 8 bytes on 64-bit ✿ ʕ •ᴥ•ʔ
    static const int kFields[] = {kSpellName, kSpellSubtext, kSpellAura, kSpellDesc};
    if (field < 4) {
        const char* text = spell_desc_ptr(storage, kFields[field]);
        if (!text) {
            return 0;
        }
        g_push_str(state, text);
        return 1;
    }
    SpellParseFn parse = spell_desc_bind().parse;
    if (!parse) {
        return 0;
    }
    char buffer[0x800];
    int use_aura = field == 4 ? 1 : 0;
    parse(storage, buffer, 0x800, 0, 0, 0, use_aura, 1, 0);
    g_push_str(state, buffer);
    return 1;
}

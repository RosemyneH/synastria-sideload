constexpr int kHighestSpellLimit = 0x1fbd0;
constexpr uint32_t kHighestSpellRecSize = 0x2b8;
constexpr int kHighestSpellName = 0x220;

using HighestSpellByNameFn = int (*)(const char*, int*);
using HighestDecompressFn = uint8_t* (*)(const uint8_t*, uint32_t, uint8_t*);

struct HighestSpellBind {
    const uint8_t* db = nullptr;
    const int32_t* compress = nullptr;
    HighestDecompressFn decompress = nullptr;
    HighestSpellByNameFn by_name = nullptr;
};

HighestSpellBind& highest_spell_bind() {
    static HighestSpellBind bind;
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
            bind.decompress = reinterpret_cast<HighestDecompressFn>(found);
        }
        found = nullptr;
        if (syn_resolve("_Z16GetSpellIdByNamePKcPi", &found)) {
            bind.by_name = reinterpret_cast<HighestSpellByNameFn>(found);
        }
    }
    return bind;
}

int highest_spell_arg(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min) || !(value < k_max)) {
        return 0;
    }
    double rounded = value >= 0.0
        ? static_cast<double>(static_cast<long long>(value + 0.5))
        : static_cast<double>(static_cast<long long>(value - 0.5));
    if (!(rounded > k_min) || !(rounded < k_max)) {
        return 0;
    }
    int id = static_cast<int>(rounded);
    if (id <= 0 || id >= kHighestSpellLimit) {
        return 0;
    }
    return id;
}

const char* highest_spell_name(int id) {
    const HighestSpellBind& bind = highest_spell_bind();
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
    const uint8_t* row = raw;
    alignas(8) uint8_t storage[kHighestSpellRecSize];
    if (bind.compress && *bind.compress) {
        if (!bind.decompress) {
            return nullptr;
        }
        bind.decompress(raw, kHighestSpellRecSize, storage);
        row = storage;
    }
    const char* name = nullptr;
    memcpy(&name, row + kHighestSpellName, sizeof(name));
    return name && name[0] ? name : nullptr;
}

// ʕ •ᴥ•ʔ✿ spell id in, highest learned rank of that name out ✿ ʕ •ᴥ•ʔ
int native_get_highest_spell(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int id = highest_spell_arg(g_to_num(state, 1));
    if (!id) {
        return 0;
    }
    const char* name = highest_spell_name(id);
    HighestSpellByNameFn by_name = highest_spell_bind().by_name;
    if (!name || !by_name) {
        return 0;
    }
    int pet = 0;
    int learned = by_name(name, &pet);
    if (!learned) {
        return 0;
    }
    g_push_num(state, learned);
    return 1;
}

constexpr int reagents_limit = 0x1fbd0;
constexpr uint32_t reagents_row_size = 0x2b8;
constexpr int reagents_attr = 0x10;
// ʕ •ᴥ•ʔ✿ ids and counts sit before the four name pointers, so the 32-bit slots stay put ✿ ʕ •ᴥ•ʔ
constexpr int reagents_item = 0xd0;
constexpr int reagents_count = 0xf0;
constexpr uint32_t reagents_trade = 0x20;

using reagents_table_fn = void (*)(void*, int, int);
using reagents_raw_fn = void (*)(void*, int);
using reagents_check_fn = int (*)(void*, int);
using reagents_decompress_fn = uint8_t* (*)(const uint8_t*, uint32_t, uint8_t*);

struct reagents_db {
    const uint8_t* spells = nullptr;
    const int32_t* compress = nullptr;
    reagents_decompress_fn decompress = nullptr;
    reagents_table_fn table = nullptr;
    reagents_raw_fn rawset = nullptr;
    reagents_check_fn check = nullptr;
};

reagents_db& reagents_bind() {
    static reagents_db bind;
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
        bind.decompress = reinterpret_cast<reagents_decompress_fn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_createtable", &found)) {
        bind.table = reinterpret_cast<reagents_table_fn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_rawset", &found)) {
        bind.rawset = reinterpret_cast<reagents_raw_fn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_checkstack", &found)) {
        bind.check = reinterpret_cast<reagents_check_fn>(found);
    }
    return bind;
}

double reagents_round(double value) {
    if (value >= 0.0) {
        return static_cast<double>(static_cast<long long>(value + 0.5));
    }
    return static_cast<double>(static_cast<long long>(value - 0.5));
}

bool reagents_spell_id(double value, int* out) {
    if (!(value > -2147483648.0) || !(value < 2147483647.0)) {
        return false;
    }
    double rounded = reagents_round(value);
    if (!(rounded > -2147483648.0) || !(rounded < 2147483647.0)) {
        return false;
    }
    int id = static_cast<int>(rounded);
    if (id < 0 || id >= reagents_limit) {
        return false;
    }
    *out = id;
    return true;
}

int32_t reagents_i32(const uint8_t* row, int offset) {
    int32_t value = 0;
    memcpy(&value, row + offset, 4);
    return value;
}

const uint8_t* reagents_spell(int id, uint8_t* storage) {
    const reagents_db& bind = reagents_bind();
    if (!bind.spells) {
        return nullptr;
    }
    int32_t max_id = reagents_i32(bind.spells, 0x10);
    int32_t min_id = reagents_i32(bind.spells, 0x14);
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
        bind.decompress(raw, reagents_row_size, storage);
        return storage;
    }
    return raw;
}

// ʕ •ᴥ•ʔ✿ one spell id; a table of reagent item id to count for tradeskill spells ✿ ʕ •ᴥ•ʔ
int native_get_profession_recipe_reagents(void* state) {
    const reagents_db& bind = reagents_bind();
    if (!bind.table || !bind.rawset || !g_push_num || !g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    int id = 0;
    if (!reagents_spell_id(g_to_num(state, 1), &id)) {
        return 0;
    }
    alignas(8) uint8_t storage[reagents_row_size];
    const uint8_t* spell = reagents_spell(id, storage);
    if (!spell || (static_cast<uint8_t>(reagents_i32(spell, reagents_attr)) & reagents_trade) == 0) {
        return 0;
    }
    bind.table(state, 0, 0);
    for (int slot = 0; slot < 8; ++slot) {
        uint32_t item = static_cast<uint32_t>(reagents_i32(spell, reagents_item + slot * 4));
        uint32_t count = static_cast<uint32_t>(reagents_i32(spell, reagents_count + slot * 4));
        if (!item || !count) {
            continue;
        }
        if (bind.check) {
            bind.check(state, 2);
        }
        g_push_num(state, static_cast<double>(item));
        g_push_num(state, static_cast<double>(count));
        bind.rawset(state, -3);
    }
    return 1;
}

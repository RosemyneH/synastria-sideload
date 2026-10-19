constexpr int kSpellToolsLimit = 0x1fbd0;
constexpr uint32_t kSpellToolsRecSize = 0x2b8;
constexpr int kSpellToolsFocus = 0x48;
constexpr int kSpellToolsTotem = 0xc8;
// ʕ •ᴥ•ʔ✿ four 8-byte strings at 0x220 shift the 32-bit tail by 0x10 ✿ ʕ •ᴥ•ʔ
constexpr int kSpellToolsTotemCat = 0x288;
constexpr int kSpellToolsArea = 0x290;
constexpr int kSpellToolsFocusName = 8;
constexpr int kSpellToolsCatName = 8;
constexpr int kSpellToolsAreaName = 0x30;
constexpr int kSpellToolsGroupArea = 4;

using SpellToolsDecompressFn = uint8_t* (*)(const uint8_t*, uint32_t, uint8_t*);

struct SpellToolsBind {
    const uint8_t* spells = nullptr;
    const int32_t* compress = nullptr;
    SpellToolsDecompressFn decompress = nullptr;
    const uint8_t* focus = nullptr;
    const uint8_t* groups = nullptr;
    const uint8_t* areas = nullptr;
    const uint8_t* categories = nullptr;
};

SpellToolsBind& spell_tools_bind() {
    static SpellToolsBind bind;
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
            bind.decompress = reinterpret_cast<SpellToolsDecompressFn>(found);
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
    }
    return bind;
}

bool spell_tools_int(double value, int* out) {
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

const uint8_t* spell_tools_spell(int id, uint8_t* storage) {
    const SpellToolsBind& bind = spell_tools_bind();
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
        bind.decompress(raw, kSpellToolsRecSize, storage);
        return storage;
    }
    return raw;
}

const uint8_t* spell_tools_record(const uint8_t* table, int id) {
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

int32_t spell_tools_i32(const uint8_t* row, int offset) {
    int32_t value = 0;
    memcpy(&value, row + offset, 4);
    return value;
}

const char* spell_tools_field(const uint8_t* row, int offset) {
    if (!row) {
        return nullptr;
    }
    const char* text = nullptr;
    memcpy(&text, row + offset, sizeof(text));
    return text;
}

void spell_tools_name(void* state, const char* text) {
    if (text) {
        g_push_str(state, text);
    } else {
        g_push_nil(state);
    }
}

void spell_tools_named(void* state, int32_t id, const uint8_t* table, int name_off) {
    if (!id) {
        g_push_num(state, 0);
        g_push_nil(state);
        return;
    }
    g_push_num(state, static_cast<double>(static_cast<uint32_t>(id)));
    spell_tools_name(state, spell_tools_field(spell_tools_record(table, id), name_off));
}

void spell_tools_area(void* state, int32_t id) {
    if (id <= 0) {
        g_push_num(state, 0);
        g_push_nil(state);
        return;
    }
    g_push_num(state, static_cast<double>(static_cast<uint32_t>(id)));
    const uint8_t* group = spell_tools_record(spell_tools_bind().groups, id);
    const char* name = nullptr;
    if (group) {
        int area = spell_tools_i32(group, kSpellToolsGroupArea);
        name = spell_tools_field(spell_tools_record(spell_tools_bind().areas, area), kSpellToolsAreaName);
    }
    spell_tools_name(state, name);
}

void spell_tools_totem(void* state, int32_t id) {
    if (!id) {
        g_push_num(state, 0);
        g_push_nil(state);
        return;
    }
    g_push_num(state, static_cast<double>(id));
    if (static_cast<uint32_t>(id) > 0xffffu) {
        g_push_nil(state);
        return;
    }
    const ItemRow* item = item_row(id);
    spell_tools_name(state, item ? item->name.c_str() : nullptr);
}

// ʕ •ᴥ•ʔ✿ spell id; focus id/name, area id/name, two totem id/name pairs, two totem-category id/name pairs ✿ ʕ •ᴥ•ʔ
int native_get_spell_tools(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_push_str || !g_push_nil || !g_is_num(state, 1)) {
        return 0;
    }
    int id = 0;
    if (!spell_tools_int(g_to_num(state, 1), &id) || id <= 0 || id >= kSpellToolsLimit) {
        return 0;
    }
    alignas(8) uint8_t storage[kSpellToolsRecSize];
    const uint8_t* row = spell_tools_spell(id, storage);
    if (!row) {
        return 0;
    }
    spell_tools_named(state, spell_tools_i32(row, kSpellToolsFocus), spell_tools_bind().focus, kSpellToolsFocusName);
    spell_tools_area(state, spell_tools_i32(row, kSpellToolsArea));
    spell_tools_totem(state, spell_tools_i32(row, kSpellToolsTotem));
    spell_tools_totem(state, spell_tools_i32(row, kSpellToolsTotem + 4));
    spell_tools_named(state, spell_tools_i32(row, kSpellToolsTotemCat), spell_tools_bind().categories, kSpellToolsCatName);
    spell_tools_named(state, spell_tools_i32(row, kSpellToolsTotemCat + 4), spell_tools_bind().categories, kSpellToolsCatName);
    return 12;
}

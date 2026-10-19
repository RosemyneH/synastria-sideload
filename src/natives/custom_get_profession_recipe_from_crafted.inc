constexpr int kCraftedSpellLast = 0x1fbcf;
constexpr uint32_t kCraftedSpellSize = 0x2b8;
constexpr int kCraftedAttr = 0x10;
constexpr int kCraftedEffect = 0x11c;
constexpr int kCraftedItemType = 0x1ac;
constexpr uint32_t kCraftedTradeBit = 0x20;

using CraftedDecompressFn = uint8_t* (*)(const uint8_t*, uint32_t, uint8_t*);

struct CraftedBind {
    const uint8_t* spells = nullptr;
    const int32_t* compress = nullptr;
    CraftedDecompressFn decompress = nullptr;
};

CraftedBind& crafted_bind() {
    static CraftedBind crafted_bind_slot;
    static bool crafted_bind_ready = false;
    if (crafted_bind_ready) {
        return crafted_bind_slot;
    }
    crafted_bind_ready = true;
    void* found = nullptr;
    if (syn_resolve("g_spellDB", &found)) {
        crafted_bind_slot.spells = static_cast<const uint8_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("g_clientDbPerformCompress", &found)) {
        crafted_bind_slot.compress = static_cast<const int32_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("_Z13DecompressRowPKhjPh", &found)) {
        crafted_bind_slot.decompress = reinterpret_cast<CraftedDecompressFn>(found);
    }
    return crafted_bind_slot;
}

int32_t crafted_i32(const uint8_t* row, int offset) {
    int32_t value = 0;
    memcpy(&value, row + offset, 4);
    return value;
}

int crafted_arg(void* state) {
    double value = g_to_num(state, 1);
    if (!(value > -2147483648.0) || !(value < 2147483647.0)) {
        return 0x7fffffff;
    }
    double rounded = value >= 0.0
        ? static_cast<double>(static_cast<long long>(value + 0.5))
        : static_cast<double>(static_cast<long long>(value - 0.5));
    if (!(rounded > -2147483648.0) || !(rounded < 2147483647.0)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

const uint8_t* crafted_spell(int id, uint8_t* storage) {
    const CraftedBind& bind = crafted_bind();
    if (!bind.spells) {
        return nullptr;
    }
    int32_t max_id = crafted_i32(bind.spells, 0x10);
    int32_t min_id = crafted_i32(bind.spells, 0x14);
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
        bind.decompress(raw, kCraftedSpellSize, storage);
        return storage;
    }
    return raw;
}

bool crafted_creates(int32_t effect) {
    return effect == 0x18 || effect == 0x9d || effect == 0x3b;
}

std::map<uint32_t, uint32_t>& crafted_cache() {
    static std::map<uint32_t, uint32_t> crafted_cache_map;
    return crafted_cache_map;
}

void crafted_fill() {
    std::map<uint32_t, uint32_t>& cache = crafted_cache();
    // ʕ •ᴥ•ʔ✿ rescan while no recipe has been stored; the lowest spell id keeps the item ✿ ʕ •ᴥ•ʔ
    if (!cache.empty()) {
        return;
    }
    alignas(8) uint8_t storage[kCraftedSpellSize];
    for (int id = 1; id <= kCraftedSpellLast; ++id) {
        const uint8_t* row = crafted_spell(id, storage);
        if (!row) {
            continue;
        }
        if ((static_cast<uint32_t>(crafted_i32(row, kCraftedAttr)) & kCraftedTradeBit) == 0) {
            continue;
        }
        if (!crafted_creates(crafted_i32(row, kCraftedEffect))) {
            continue;
        }
        uint32_t item = static_cast<uint32_t>(crafted_i32(row, kCraftedItemType));
        if (!item || cache.find(item) != cache.end()) {
            continue;
        }
        cache.emplace(item, static_cast<uint32_t>(crafted_i32(row, 0)));
    }
}

// ʕ •ᴥ•ʔ✿ item id; spell id that crafts it ✿ ʕ •ᴥ•ʔ
int native_get_profession_recipe_from_crafted(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int item = crafted_arg(state);
    if (item <= 0 || static_cast<uint32_t>(item) > 0xffffu || !item_row(item)) {
        return 0;
    }
    crafted_fill();
    const std::map<uint32_t, uint32_t>& cache = crafted_cache();
    auto found = cache.find(static_cast<uint32_t>(item));
    if (found == cache.end()) {
        return 0;
    }
    g_push_num(state, static_cast<double>(found->second));
    return 1;
}

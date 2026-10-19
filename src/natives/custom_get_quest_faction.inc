std::map<uint32_t, uint8_t> quest_faction_rows;
bool quest_faction_ready = false;

// ʕ •ᴥ•ʔ✿ Id delta from the loader: the first byte is the high 7-bit group. ✿ ʕ •ᴥ•ʔ
uint32_t quest_faction_delta(BinReader& in) {
    uint8_t byte = in.u8();
    uint32_t value = byte & 0x7f;
    for (int i = 0; i < 4 && (byte & 0x80) != 0; ++i) {
        byte = in.u8();
        value = (value << 7) | (byte & 0x7f);
    }
    return value;
}

void quest_faction_cstr(BinReader& in) {
    while (in.ok) {
        if (in.u8() == 0) {
            return;
        }
    }
}

void quest_faction_named(BinReader& in, bool extra) {
    quest_faction_cstr(in);
    if (in.u16() != 0xffff) {
        in.u8();
    }
    if (!extra) {
        return;
    }
    uint8_t mask = in.u8();
    for (int bit = 0; bit < 3; ++bit) {
        if (mask & (1 << bit)) {
            in.u16();
        }
    }
}

void quest_faction_words(BinReader& in) {
    uint8_t count = in.u8();
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        in.u16();
    }
}

void quest_faction_skip_row(BinReader& in) {
    uint32_t bits = in.u32();
    if (bits & 0x1) {
        in.u16();
    }
    if (bits & 0x8) {
        in.u16();
    }
    if (bits & 0x10) {
        in.u16();
    }
    if (bits & 0x20) {
        in.u8();
    }
    if (bits & 0x40) {
        in.u8();
    }
    if (bits & 0x80) {
        in.u16();
        in.u16();
    }
    if (bits & 0x100) {
        in.u32();
    }
    if (bits & 0x200) {
        in.u32();
    }
    if (bits & 0x400) {
        in.u32();
    }
    if (bits & 0x800) {
        quest_faction_words(in);
    }
    if (bits & 0x1000) {
        in.u16();
    }
    if (bits & 0x2000) {
        quest_faction_words(in);
    }
    if (bits & 0x100000) {
        quest_faction_words(in);
    }
    if (bits & 0x200000) {
        quest_faction_words(in);
    }
    if (bits & 0x400000) {
        quest_faction_words(in);
    }
    if (bits & 0x800000) {
        quest_faction_words(in);
    }
}

void load_quest_faction() {
    if (quest_faction_ready || !g_root[0]) {
        return;
    }
    quest_faction_ready = true;
    char path[1024];
    snprintf(path, sizeof(path), "%s/WoWExtItemLoc.bin", g_root);
    size_t size = 0;
    uint8_t* bytes = read_alloc(path, &size);
    if (!bytes) {
        return;
    }
    BinReader in{bytes, size, 0, true};
    uint32_t version = in.u32();
    if (version < 1 || version > 5) {
        free(bytes);
        return;
    }
    in.u32();
    uint8_t bitmap[0x2000];
    if (!in.take(bitmap, sizeof(bitmap))) {
        free(bytes);
        return;
    }
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((bitmap[id >> 3] & (1 << (id & 7))) == 0) {
            continue;
        }
        quest_faction_named(in, version >= 2);
    }
    uint32_t creatures = in.u32();
    for (uint32_t i = 0; i < creatures && in.ok; ++i) {
        quest_faction_delta(in);
        quest_faction_named(in, false);
    }
    uint32_t quests = in.u32();
    if (version >= 5) {
        in.u32();
    }
    uint32_t running = 0;
    for (uint32_t i = 0; i < quests && in.ok; ++i) {
        running += quest_faction_delta(in);
        quest_faction_cstr(in);
        in.u16();
        uint8_t flags = 0;
        if (version >= 3) {
            flags = in.u8();
            if (flags & 0x4) {
                quest_faction_skip_row(in);
            }
        }
        uint8_t faction = flags & 0x3;
        if (faction == 1 || faction == 2) {
            quest_faction_rows[running | 0x2000000u] = faction;
        }
    }
    log_line("WoWExtItemLoc.bin quest faction %zu", quest_faction_rows.size());
    free(bytes);
}

int quest_faction_int(double value) {
    if (!(value > -2147483648.0)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(value < 2147483647.0)) {
        return 0x7fffffff;
    }
    long long rounded = value >= 0.0
        ? static_cast<long long>(value + 0.5)
        : static_cast<long long>(value - 0.5);
    if (rounded <= static_cast<long long>(static_cast<int>(0x80000000u))) {
        return static_cast<int>(0x80000000u);
    }
    if (rounded >= 2147483647LL) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

// ʕ •ᴥ•ʔ✿ Flags byte bits 0-1. 1 Alliance, 2 Horde; 0 and 3 push 0. Always one number. ✿ ʕ •ᴥ•ʔ
int native_get_quest_faction(void* state) {
    load_quest_faction();
    double pushed = 0.0;
    if (g_is_num && g_to_num && g_is_num(state, 1)) {
        uint32_t key = static_cast<uint32_t>(quest_faction_int(g_to_num(state, 1))) | 0x2000000u;
        auto found = quest_faction_rows.find(key);
        if (found != quest_faction_rows.end()) {
            pushed = found->second;
        }
    }
    if (!g_push_num) {
        return 0;
    }
    g_push_num(state, pushed);
    return 1;
}

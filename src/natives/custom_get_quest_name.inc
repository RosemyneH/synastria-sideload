std::map<int, std::string> g_quest_names;
bool g_quest_names_ready = false;

uint32_t quest_varint(BinReader& in) {
    uint32_t value = 0;
    uint8_t byte = 0;
    do {
        byte = in.u8();
        value = (value << 7) | (byte & 0x7f);
    } while (in.ok && (byte & 0x80) != 0);
    return value;
}

std::string quest_cstr(BinReader& in) {
    std::string text;
    while (in.ok) {
        uint8_t byte = in.u8();
        if (!in.ok || byte == 0) {
            break;
        }
        text.push_back(static_cast<char>(byte));
    }
    return text;
}

void skip_u16_list(BinReader& in) {
    uint8_t count = in.u8();
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        in.u16();
    }
}

void skip_quest_extra(BinReader& in) {
    uint32_t word = in.u32();
    if (word & 0x1) {
        in.u16();
    }
    if (word & 0x8) {
        in.u16();
    }
    if (word & 0x10) {
        in.u16();
    }
    if (word & 0x20) {
        in.u8();
    }
    if (word & 0x40) {
        in.u8();
    }
    if (word & 0x80) {
        in.u16();
        in.u16();
    }
    if (word & 0x100) {
        in.u32();
    }
    if (word & 0x200) {
        in.u32();
    }
    if (word & 0x400) {
        in.u32();
    }
    if (word & 0x800) {
        skip_u16_list(in);
    }
    if (word & 0x1000) {
        in.u16();
    }
    if (word & 0x2000) {
        skip_u16_list(in);
    }
    if (word & 0x100000) {
        skip_u16_list(in);
    }
    if (word & 0x200000) {
        skip_u16_list(in);
    }
    if (word & 0x400000) {
        skip_u16_list(in);
    }
    if (word & 0x800000) {
        skip_u16_list(in);
    }
}

void skip_name_tail(BinReader& in) {
    if (in.u16() != 0xffff) {
        in.u8();
    }
}

void load_quest_names() {
    if (g_quest_names_ready || !g_root[0]) {
        return;
    }
    g_quest_names_ready = true;
    char path[1024];
    snprintf(path, sizeof(path), "%s/WoWExtItemLoc.bin", g_root);
    size_t size = 0;
    uint8_t* bytes = read_alloc(path, &size);
    if (!bytes) {
        return;
    }
    BinReader in{bytes, size, 0, true};
    uint32_t version = in.u32();
    in.u32();
    if (version < 1 || version > 5) {
        free(bytes);
        return;
    }
    uint8_t bitmap[0x2000];
    if (!in.take(bitmap, sizeof(bitmap))) {
        free(bytes);
        return;
    }
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((bitmap[id >> 3] & (1 << (id & 7))) == 0) {
            continue;
        }
        quest_cstr(in);
        skip_name_tail(in);
        if (version >= 2) {
            uint8_t flags = in.u8();
            for (int bit = 0; bit < 3; ++bit) {
                if (flags & (1 << bit)) {
                    in.u16();
                }
            }
        }
    }
    uint32_t zones = in.u32();
    for (uint32_t i = 0; i < zones && in.ok; ++i) {
        quest_varint(in);
        quest_cstr(in);
        skip_name_tail(in);
    }
    uint32_t quests = in.u32();
    if (version >= 5) {
        in.u32();
    }
    uint32_t running = 0;
    for (uint32_t i = 0; i < quests && in.ok; ++i) {
        running += quest_varint(in);
        std::string name = quest_cstr(in);
        in.u16();
        if (version >= 3) {
            uint8_t flags = in.u8();
            if (flags & 0x04) {
                skip_quest_extra(in);
            }
        }
        if (running > 0 && running < 0x10000) {
            g_quest_names[static_cast<int>(running)] = std::move(name);
        }
    }
    log_line("WoWExtItemLoc.bin quests %zu", g_quest_names.size());
    free(bytes);
}

// ʕ •ᴥ•ʔ✿ Titles are the ItemLoc quest records. A missing id leaves the stack empty. ✿ ʕ •ᴥ•ʔ
int native_get_quest_name(void* state) {
    load_quest_names();
    if (!g_is_num || !g_to_num || !g_is_num(state, 1)) {
        return 0;
    }
    double value = g_to_num(state, 1);
    if (!(value > -2147483648.0) || !(value < 2147483647.0)) {
        return 0;
    }
    long long rounded = value >= 0
        ? static_cast<long long>(value + 0.5)
        : static_cast<long long>(value - 0.5);
    if (rounded <= (-2147483647LL - 1) || rounded >= 2147483647LL) {
        return 0;
    }
    int id = static_cast<int>(rounded);
    if (id <= 0 || id >= 0x10000) {
        return 0;
    }
    auto found = g_quest_names.find(id);
    if (found == g_quest_names.end()) {
        return 0;
    }
    g_push_str(state, found->second.c_str());
    return 1;
}

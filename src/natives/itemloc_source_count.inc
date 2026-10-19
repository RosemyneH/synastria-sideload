uint32_t src_count_values[0x10000];

const uint8_t src_count_plain[0x16] = {
    0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
};

const uint8_t src_count_mode[0x16] = {
    0, 1, 2, 3, 4, 0, 3, 5, 1, 0, 3, 3, 6, 0, 0, 7, 0, 1, 3, 8, 0, 1,
};

struct SrcCountReader {
    const uint8_t* data;
    size_t size;
    size_t off;
    bool ok;

    bool take(void* dst, size_t n) {
        if (!ok || off + n > size) {
            ok = false;
            return false;
        }
        memcpy(dst, data + off, n);
        off += n;
        return true;
    }

    uint8_t u8() {
        uint8_t value = 0;
        take(&value, 1);
        return value;
    }

    uint16_t u16() {
        uint16_t value = 0;
        take(&value, 2);
        return value;
    }

    uint32_t u32() {
        uint32_t value = 0;
        take(&value, 4);
        return value;
    }

    uint32_t varint() {
        uint8_t byte = u8();
        uint32_t value = byte & 0x7fu;
        for (int left = 4; (byte & 0x80) != 0 && left > 0 && ok; --left) {
            byte = u8();
            value = (byte & 0x7fu) | (value << 7);
        }
        return value;
    }

    void cstr() {
        while (ok) {
            if (u8() == 0) {
                break;
            }
        }
    }

    void u16_list() {
        uint8_t count = u8();
        for (uint8_t i = 0; i < count && ok; ++i) {
            u16();
        }
    }

    void extra() {
        uint32_t flags = u32();
        if (flags & 0x1) {
            u16();
        }
        if (flags & 0x8) {
            u16();
        }
        if (flags & 0x10) {
            u16();
        }
        if (flags & 0x20) {
            u8();
        }
        if (flags & 0x40) {
            u8();
        }
        if (flags & 0x80) {
            u16();
            u16();
        }
        if (flags & 0x100) {
            u32();
        }
        if (flags & 0x200) {
            u32();
        }
        if (flags & 0x400) {
            u32();
        }
        if (flags & 0x800) {
            u16_list();
        }
        if (flags & 0x1000) {
            u16();
        }
        if (flags & 0x2000) {
            u16_list();
        }
        if (flags & 0x100000) {
            u16_list();
        }
        if (flags & 0x200000) {
            u16_list();
        }
        if (flags & 0x400000) {
            u16_list();
        }
        if (flags & 0x800000) {
            u16_list();
        }
    }
};

void src_count_clear() {
    memset(src_count_values, 0, sizeof(src_count_values));
}

bool src_count_kept(uint8_t kind, uint32_t id, const uint8_t* creatures, const std::map<uint32_t, uint8_t>& known) {
    if (kind > 0x15) {
        return false;
    }
    switch (src_count_mode[kind]) {
    case 0:
        return id < 0x10000 && (creatures[id >> 3] & (1u << (id & 7))) != 0;
    case 1:
        return known.find(id | 0x1000000u) != known.end();
    case 2:
        return known.find(id | 0x2000000u) != known.end();
    case 3:
    case 6:
    case 8:
        return true;
    case 5:
        // ʕ •ᴥ•ʔ✿ Wow.exe only fills the name; a placeholder still keeps the source ✿ ʕ •ᴥ•ʔ
        return true;
    default:
        return false;
    }
}

bool src_count_parse(const uint8_t* data, size_t size) {
    SrcCountReader in{data, size, 0, true};
    uint32_t version = in.u32();
    in.u32();
    if (version < 1 || version > 5) {
        return false;
    }
    uint8_t creatures[0x2000];
    if (!in.take(creatures, sizeof(creatures))) {
        return false;
    }
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((creatures[id >> 3] & (1u << (id & 7))) == 0) {
            continue;
        }
        in.cstr();
        if (in.u16() != 0xffff) {
            in.u8();
        }
        if (version >= 2) {
            uint8_t flags = in.u8();
            for (int bit = 0; bit < 3; ++bit) {
                if (flags & (1 << bit)) {
                    in.u16();
                }
            }
        }
    }
    int32_t objects = static_cast<int32_t>(in.u32());
    std::map<uint32_t, uint8_t> known;
    uint32_t running = 0;
    for (int32_t i = 0; i < objects && in.ok; ++i) {
        running += in.varint();
        in.cstr();
        if (in.u16() != 0xffff) {
            in.u8();
        }
        known[running | 0x1000000u] = 1;
    }
    int32_t quests = static_cast<int32_t>(in.u32());
    if (version >= 5) {
        in.u32();
    }
    running = 0;
    for (int32_t i = 0; i < quests && in.ok; ++i) {
        running += in.varint();
        in.cstr();
        in.u16();
        if (version >= 3) {
            uint8_t flags = in.u8();
            if (flags & 0x04) {
                in.extra();
            }
        }
        known[running | 0x2000000u] = 1;
    }
    uint8_t items[0x2000];
    if (!in.take(items, sizeof(items))) {
        return false;
    }
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((items[id >> 3] & (1u << (id & 7))) == 0) {
            continue;
        }
        uint32_t count = 0;
        uint8_t kind = in.u8();
        while (kind != 0xff && in.ok) {
            uint32_t group = in.varint();
            bool extra = kind <= 0x15 && src_count_plain[kind] == 0;
            uint32_t source = 0;
            for (uint32_t n = 0; n < group && in.ok; ++n) {
                source += in.varint();
                if (extra) {
                    in.u16();
                    in.u16();
                }
                if (src_count_kept(kind, source, creatures, known)) {
                    ++count;
                }
            }
            kind = in.u8();
        }
        src_count_values[id] = count;
    }
    return in.ok;
}

void load_itemloc_source_count(const uint8_t* data, size_t size) {
    src_count_clear();
    if (!data || size < 4 || !src_count_parse(data, size)) {
        src_count_clear();
        log_line("ItemLoc source counts rejected");
        return;
    }
    int filled = 0;
    for (uint32_t count : src_count_values) {
        if (count) {
            ++filled;
        }
    }
    log_line("ItemLoc source counts %d", filled);
}

int native_itemloc_source_count(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
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
    uint32_t count = src_count_values[id];
    if (count == 0) {
        return 0;
    }
    g_push_num(state, static_cast<double>(count));
    return 1;
}

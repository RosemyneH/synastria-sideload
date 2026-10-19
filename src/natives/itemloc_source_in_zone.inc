// ʕ •ᴥ•ʔ✿ ItemLoc counts are 7-bit groups, first byte most significant, five bytes max ✿ ʕ •ᴥ•ʔ
uint32_t src_in_zone_varint(BinReader& in) {
    uint32_t value = 0;
    for (int i = 0; i < 5 && in.ok; ++i) {
        uint8_t byte = in.u8();
        value = (value << 7) | (byte & 0x7fu);
        if ((byte & 0x80) == 0) {
            break;
        }
    }
    return value;
}

void src_in_zone_cstr(BinReader& in) {
    while (in.ok && in.u8() != 0) {
    }
}

void src_in_zone_u16_list(BinReader& in) {
    uint8_t count = in.u8();
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        in.u16();
    }
}

void src_in_zone_quest_extra(BinReader& in) {
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
        src_in_zone_u16_list(in);
    }
    if (word & 0x1000) {
        in.u16();
    }
    if (word & 0x2000) {
        src_in_zone_u16_list(in);
    }
    if (word & 0x100000) {
        src_in_zone_u16_list(in);
    }
    if (word & 0x200000) {
        src_in_zone_u16_list(in);
    }
    if (word & 0x400000) {
        src_in_zone_u16_list(in);
    }
    if (word & 0x800000) {
        src_in_zone_u16_list(in);
    }
}

struct SrcInZoneRef {
    uint16_t slot;
    uint8_t type;
};

int32_t src_in_zone_header = 0;
bool src_in_zone_ready = false;
uint16_t src_in_zone_top = 0;
std::vector<uint16_t> src_in_zone_zone;
std::vector<char> src_in_zone_on;
std::map<uint32_t, uint16_t> src_in_zone_keys;
std::vector<std::vector<SrcInZoneRef>> src_in_zone_items;
const uint8_t* src_in_zone_areas = nullptr;
const uint8_t* src_in_zone_maps = nullptr;

const uint8_t* src_in_zone_db(const char* symbol) {
    void* found = nullptr;
    if (!syn_resolve(symbol, &found)) {
        return nullptr;
    }
    return static_cast<const uint8_t*>(found);
}

bool src_in_zone_row(const uint8_t* table, int id, const uint8_t** row) {
    if (!table || id < 0) {
        return false;
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&max_id, table + 0x10, 4);
    memcpy(&min_id, table + 0x14, 4);
    if (id < min_id || id > max_id) {
        return false;
    }
    void** by_id = nullptr;
    memcpy(&by_id, table + 0x30, sizeof(by_id));
    if (!by_id) {
        return false;
    }
    *row = static_cast<const uint8_t*>(by_id[id - min_id]);
    return *row != nullptr;
}

int src_in_zone_root(int id, int depth) {
    const uint8_t* row = nullptr;
    if (!src_in_zone_row(src_in_zone_areas, id, &row)) {
        return 0;
    }
    int32_t parent = 0;
    memcpy(&parent, row + 8, 4);
    if (parent == 0) {
        return id;
    }
    if (depth >= 10) {
        return 0;
    }
    return src_in_zone_root(parent, depth + 1);
}

uint16_t src_in_zone_make(uint32_t key, uint16_t zone) {
    if (src_in_zone_top == 0xffff) {
        return 0;
    }
    uint16_t slot = static_cast<uint16_t>(src_in_zone_top + 1);
    src_in_zone_top = slot;
    if (src_in_zone_zone.size() <= slot) {
        src_in_zone_zone.resize(slot + 1);
        src_in_zone_on.resize(slot + 1);
    }
    src_in_zone_zone[slot] = zone;
    src_in_zone_on[slot] = 1;
    src_in_zone_keys[key] = slot;
    return slot;
}

bool src_in_zone_map_named(uint32_t id) {
    if (!src_in_zone_maps) {
        return true;
    }
    int map_id = static_cast<int>(id & 0xffffu);
    if (map_id & 0x8000) {
        map_id &= 0x3ff;
    }
    const uint8_t* row = nullptr;
    if (!src_in_zone_row(src_in_zone_maps, map_id, &row)) {
        return false;
    }
    const char* name = nullptr;
    memcpy(&name, row + 32, sizeof(name));
    return name && name[0];
}

uint16_t src_in_zone_named(uint32_t id, uint16_t zone) {
    uint32_t key = id | 0x5000000u;
    auto found = src_in_zone_keys.find(key);
    if (found != src_in_zone_keys.end()) {
        return found->second;
    }
    if (!src_in_zone_map_named(id)) {
        return 0;
    }
    int area_id = static_cast<int>(id & 0x3fffu);
    const uint8_t* row = nullptr;
    if (src_in_zone_row(src_in_zone_areas, area_id, &row)) {
        int32_t continent = 0;
        memcpy(&continent, row + 4, 4);
        if (continent == 0 || continent == 1 || continent == 0x212 || continent == 0x23b) {
            int32_t parent = 0;
            memcpy(&parent, row + 8, 4);
            if (parent != 0) {
                int root = src_in_zone_root(parent, 1);
                if (root > 0 && root <= 0xffff) {
                    zone = static_cast<uint16_t>(root);
                }
            }
        } else {
            zone = static_cast<uint16_t>(continent | 0x8000);
        }
    }
    return src_in_zone_make(key, zone);
}

// ʕ •ᴥ•ʔ✿ source type -> hash tag. 7 drops the source. 3/5/6/8 mint a slot on a miss ✿ ʕ •ᴥ•ʔ
uint16_t src_in_zone_find(uint8_t type, uint32_t id) {
    static const uint8_t k_case[] = {
        0, 1, 2, 3, 4, 0, 3, 5, 1, 0, 3, 3, 6, 0, 0, 7, 0, 1, 3, 8, 0, 1,
    };
    if (type > 0x15) {
        return 0;
    }
    uint32_t key = id;
    switch (k_case[type]) {
    case 1:
        key |= 0x1000000u;
        break;
    case 2:
        key |= 0x2000000u;
        break;
    case 3:
        key |= 0x3000000u;
        break;
    case 4:
        key |= 0x4000000u;
        break;
    case 5:
        return src_in_zone_named(id, 0xffff);
    case 6:
        key |= 0x6000000u;
        break;
    case 7:
        return 0;
    case 8:
        key |= 0x15000000u;
        break;
    default:
        break;
    }
    auto found = src_in_zone_keys.find(key);
    if (found != src_in_zone_keys.end()) {
        return found->second;
    }
    uint8_t kind = k_case[type];
    if (kind == 3 || kind == 6 || kind == 8) {
        return src_in_zone_make(key, 0xffff);
    }
    return 0;
}

void load_itemloc_source_in_zone(const uint8_t* data, size_t size) {
    src_in_zone_ready = true;
    src_in_zone_header = 0;
    src_in_zone_top = 0;
    src_in_zone_zone.clear();
    src_in_zone_on.clear();
    src_in_zone_keys.clear();
    src_in_zone_items.clear();
    src_in_zone_areas = src_in_zone_db("g_areaTableDB");
    src_in_zone_maps = src_in_zone_db("g_mapDB");
    if (!data || size < 8) {
        return;
    }
    BinReader in{data, size, 0, true};
    uint32_t version = in.u32();
    int32_t header = static_cast<int32_t>(in.u32());
    if (version < 1 || version > 5 || !in.ok) {
        return;
    }
    src_in_zone_header = header;
    uint8_t bitmap[0x2000];
    if (!in.take(bitmap, sizeof(bitmap))) {
        return;
    }
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((bitmap[id >> 3] & (1 << (id & 7))) == 0) {
            continue;
        }
        src_in_zone_cstr(in);
        uint16_t zone = in.u16();
        if (zone != 0xffff) {
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
        src_in_zone_make(static_cast<uint32_t>(id), zone);
    }
    uint32_t zones = in.u32();
    uint32_t running = 0;
    for (uint32_t i = 0; i < zones && in.ok; ++i) {
        running += src_in_zone_varint(in);
        src_in_zone_cstr(in);
        uint16_t zone = in.u16();
        if (zone != 0xffff) {
            in.u8();
        }
        src_in_zone_make(running | 0x1000000u, zone);
    }
    uint32_t quests = in.u32();
    if (version >= 5) {
        in.u32();
    }
    running = 0;
    for (uint32_t i = 0; i < quests && in.ok; ++i) {
        running += src_in_zone_varint(in);
        src_in_zone_cstr(in);
        uint16_t zone = in.u16();
        if (version >= 3) {
            uint8_t flags = in.u8();
            if (flags & 0x04) {
                src_in_zone_quest_extra(in);
            }
        }
        src_in_zone_make(running | 0x2000000u, zone);
    }
    if (!in.take(bitmap, sizeof(bitmap))) {
        return;
    }
    static const uint8_t k_extra[] = {
        1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
    };
    src_in_zone_items.assign(0x10000, {});
    uint32_t kept = 0;
    for (int item = 0; item < 0x10000 && in.ok; ++item) {
        if ((bitmap[item >> 3] & (1 << (item & 7))) == 0) {
            continue;
        }
        while (in.ok) {
            uint8_t type = in.u8();
            if (!in.ok || type == 0xff) {
                break;
            }
            uint32_t count = src_in_zone_varint(in);
            bool extra = type <= 0x15 && k_extra[type];
            uint32_t acc = 0;
            for (uint32_t n = 0; n < count && in.ok; ++n) {
                acc += src_in_zone_varint(in);
                if (extra) {
                    in.u16();
                    in.u16();
                }
                uint16_t slot = src_in_zone_find(type, acc);
                if (slot) {
                    src_in_zone_items[static_cast<size_t>(item)].push_back({slot, type});
                    ++kept;
                }
            }
        }
    }
    log_line("WoWExtItemLoc.bin sources %u slots %u", kept, src_in_zone_top);
}

void src_in_zone_ensure() {
    if (src_in_zone_ready || !g_root[0]) {
        return;
    }
    char path[1024];
    snprintf(path, sizeof(path), "%s/WoWExtItemLoc.bin", g_root);
    size_t size = 0;
    uint8_t* bytes = read_alloc(path, &size);
    if (!bytes) {
        src_in_zone_ready = true;
        return;
    }
    load_itemloc_source_in_zone(bytes, size);
    free(bytes);
}

int src_in_zone_to_int(double n) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(n > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(n < k_max)) {
        return 0x7fffffff;
    }
    double whole = static_cast<double>(static_cast<int>(n));
    if (n < 0.0 && whole != n) {
        n = whole - 1.0;
    } else {
        n = whole;
    }
    if (!(n > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(n < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(n);
}

bool src_in_zone_continent(int area, int32_t* continent) {
    const uint8_t* row = nullptr;
    if (!src_in_zone_row(src_in_zone_areas, area, &row)) {
        return false;
    }
    memcpy(continent, row + 4, 4);
    return true;
}

bool src_in_zone_match(uint32_t left, uint32_t right) {
    if (static_cast<uint16_t>(left) >= 0xfff0 || static_cast<uint16_t>(right) >= 0xfff0) {
        return false;
    }
    if (static_cast<int16_t>(left) >= 0) {
        if (static_cast<int16_t>(right) < 0) {
            return false;
        }
        return ((left ^ right) & 0x3fffu) == 0;
    }
    if (static_cast<int16_t>(right) < 0) {
        if (((left ^ right) & 0x3ffu) != 0) {
            return false;
        }
        uint32_t diff = (left >> 10) & 0xfu;
        if (diff == 0) {
            return true;
        }
        return ((right >> 10) & 0xfu) == diff;
    }
    int32_t continent = 0;
    if (!src_in_zone_continent(static_cast<int>(right & 0x3fffu), &continent)) {
        return false;
    }
    if (continent != static_cast<int32_t>(left & 0x3ffu)) {
        return false;
    }
    return ((left >> 10) & 0xfu) == 0;
}

int native_itemloc_source_in_zone(void* state) {
    src_in_zone_ensure();
    if (src_in_zone_header <= 0) {
        return 0;
    }
    if (!g_is_num || !g_to_num || !g_push_num) {
        return 0;
    }
    if (!g_is_num(state, 1) || !g_is_num(state, 2) || !g_is_num(state, 3)) {
        return 0;
    }
    int item = src_in_zone_to_int(g_to_num(state, 1));
    int index = src_in_zone_to_int(g_to_num(state, 2));
    int zone = src_in_zone_to_int(g_to_num(state, 3));
    if (item <= 0 || item >= 0x10000 || zone <= 0 || zone > 0xffff) {
        return 0;
    }
    int at = index - 1;
    if (at < 0) {
        return 0;
    }
    if (src_in_zone_items.size() <= static_cast<size_t>(item)) {
        return 0;
    }
    const std::vector<SrcInZoneRef>& sources = src_in_zone_items[static_cast<size_t>(item)];
    if (at >= static_cast<int>(sources.size())) {
        return 0;
    }
    const SrcInZoneRef& source = sources[static_cast<size_t>(at)];
    if (!source.slot || source.slot >= src_in_zone_on.size() || !src_in_zone_on[source.slot]) {
        return 0;
    }
    uint32_t body = src_in_zone_zone[source.slot];
    // ʕ •ᴥ•ʔ✿ types 0x14 and 0x15 rewrite a negative zone below 0xfff0: clear bits 11-12, set 10 and 13 ✿ ʕ •ᴥ•ʔ
    if (source.type == 0x14 || source.type == 0x15) {
        uint16_t word = static_cast<uint16_t>(body);
        if (static_cast<int16_t>(word) < 0 && word < 0xfff0) {
            body = (word & 0xe7ffu) | 0x2400u;
        }
    }
    if (!src_in_zone_match(static_cast<uint32_t>(zone), body)) {
        return 0;
    }
    g_push_num(state, 1.0);
    return 1;
}

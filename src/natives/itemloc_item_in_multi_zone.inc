struct MultiZoneLink {
    uint16_t zone;
    uint8_t kind;
};

int32_t multi_zone_header = 0;
std::map<uint32_t, uint16_t> multi_zone_words;
std::vector<std::vector<MultiZoneLink>> multi_zone_items;
const uint8_t* multi_zone_areas = nullptr;
bool multi_zone_areas_bound = false;
const uint8_t multi_zone_extra[22] = {
    1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
};

// ʕ •ᴥ•ʔ✿ Counts and ids are 7-bit groups, high group first. ✿ ʕ •ᴥ•ʔ
uint32_t multi_zone_varint(BinReader& in) {
    uint32_t value = 0;
    for (int n = 0; n < 5 && in.ok; ++n) {
        uint8_t byte = in.u8();
        value = (value << 7) | (byte & 0x7f);
        if ((byte & 0x80) == 0) {
            break;
        }
    }
    return value;
}

void multi_zone_skip_str(BinReader& in) {
    while (in.ok && in.u8() != 0) {
    }
}

void multi_zone_skip_list(BinReader& in) {
    uint8_t count = in.u8();
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        in.u16();
    }
}

void multi_zone_skip_extra(BinReader& in) {
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
        multi_zone_skip_list(in);
    }
    if (word & 0x1000) {
        in.u16();
    }
    if (word & 0x2000) {
        multi_zone_skip_list(in);
    }
    if (word & 0x100000) {
        multi_zone_skip_list(in);
    }
    if (word & 0x200000) {
        multi_zone_skip_list(in);
    }
    if (word & 0x400000) {
        multi_zone_skip_list(in);
    }
    if (word & 0x800000) {
        multi_zone_skip_list(in);
    }
}

void multi_zone_bind_areas() {
    if (multi_zone_areas_bound) {
        return;
    }
    multi_zone_areas_bound = true;
    void* found = nullptr;
    if (syn_resolve("g_areaTableDB", &found)) {
        multi_zone_areas = static_cast<const uint8_t*>(found);
    }
}

const uint8_t* multi_zone_area_row(int32_t area, int32_t* min_id, int32_t* max_id) {
    if (!multi_zone_areas) {
        return nullptr;
    }
    memcpy(max_id, multi_zone_areas + 0x10, 4);
    memcpy(min_id, multi_zone_areas + 0x14, 4);
    if (area < *min_id || area > *max_id) {
        return nullptr;
    }
    void** by_id = nullptr;
    memcpy(&by_id, multi_zone_areas + 0x30, sizeof(by_id));
    if (!by_id) {
        return nullptr;
    }
    return static_cast<const uint8_t*>(by_id[area - *min_id]);
}

int32_t multi_zone_walk_area(int32_t area, int depth) {
    int32_t min_id = 0;
    int32_t max_id = 0;
    const uint8_t* row = multi_zone_area_row(area, &min_id, &max_id);
    if (!row) {
        return 0;
    }
    int32_t parent = 0;
    memcpy(&parent, row + 8, 4);
    if (parent == 0) {
        return area;
    }
    if (depth >= 10) {
        return 0;
    }
    return multi_zone_walk_area(parent, depth + 1);
}

// ʕ •ᴥ•ʔ✿ Kind 7 fills a missing record from AreaTable. Continents walk the parent at +8; other maps store map|0x8000. ✿ ʕ •ᴥ•ʔ
uint16_t multi_zone_area_word(uint32_t id) {
    multi_zone_bind_areas();
    int32_t area = static_cast<int32_t>(id & 0x3fffu);
    int32_t min_id = 0;
    int32_t max_id = 0;
    const uint8_t* row = multi_zone_area_row(area, &min_id, &max_id);
    if (!row) {
        return 0xffff;
    }
    int32_t map = 0;
    memcpy(&map, row + 4, 4);
    if (map != 0 && map != 1 && map != 0x212 && map != 0x23b) {
        return static_cast<uint16_t>(map | 0x8000);
    }
    int32_t parent = 0;
    memcpy(&parent, row + 8, 4);
    int32_t resolved = parent == 0 ? area : multi_zone_walk_area(parent, 1);
    if (resolved <= 0 || resolved > 0xffff) {
        return 0xffff;
    }
    return static_cast<uint16_t>(resolved);
}

bool multi_zone_zone_of(uint8_t kind, uint32_t id, uint16_t* zone) {
    uint32_t key = id;
    bool create = false;
    switch (kind) {
    case 0:
    case 5:
    case 9:
    case 13:
    case 14:
    case 16:
    case 20:
        break;
    case 1:
    case 8:
    case 17:
    case 21:
        key = id | 0x01000000u;
        break;
    case 2:
        key = id | 0x02000000u;
        break;
    case 4:
        key = id | 0x04000000u;
        break;
    case 3:
    case 6:
    case 10:
    case 11:
    case 18:
        key = id | 0x03000000u;
        create = true;
        break;
    case 7:
        key = id | 0x05000000u;
        create = true;
        break;
    case 12:
        key = id | 0x06000000u;
        create = true;
        break;
    case 19:
        key = id | 0x15000000u;
        create = true;
        break;
    default:
        return false;
    }
    auto found = multi_zone_words.find(key);
    if (found != multi_zone_words.end()) {
        *zone = found->second;
        return true;
    }
    if (!create) {
        return false;
    }
    uint16_t created = kind == 7 ? multi_zone_area_word(id) : 0xffff;
    multi_zone_words.emplace(key, created);
    *zone = created;
    return true;
}

uint16_t multi_zone_read_source(BinReader& in, uint32_t version) {
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
    return zone;
}

double multi_zone_floor(double n) {
    double whole = static_cast<double>(static_cast<int64_t>(n));
    if (n >= 0.0 || whole == n) {
        return whole;
    }
    return whole - 1.0;
}

int multi_zone_to_int(double n) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(n > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(n < k_max)) {
        return 0x7fffffff;
    }
    double floored = multi_zone_floor(n);
    if (!(floored > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(floored < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(floored);
}

bool multi_zone_flag_on(double n) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(n > k_min)) {
        return false;
    }
    if (!(n < k_max)) {
        return true;
    }
    double floored = multi_zone_floor(n);
    if (!(floored > k_min)) {
        return false;
    }
    if (!(floored < k_max)) {
        return true;
    }
    return static_cast<int>(floored) > 0;
}

// ʕ •ᴥ•ʔ✿ A second positive arg keeps 0xc3ff. Kinds 0x14 and 0x15 rewrite negative map ids first. ✿ ʕ •ᴥ•ʔ
bool multi_zone_differs(const std::vector<MultiZoneLink>& links, bool mask) {
    uint16_t seen = 0;
    for (const MultiZoneLink& link : links) {
        uint16_t zone = link.zone;
        if (zone == 0) {
            continue;
        }
        if ((link.kind == 0x14 || link.kind == 0x15) && static_cast<int16_t>(zone) < 0 && zone < 0xfff0) {
            zone = static_cast<uint16_t>((zone & 0xe7ff) | 0x2400);
        }
        if (mask) {
            zone = static_cast<uint16_t>(zone & 0xc3ff);
        }
        if (seen == 0) {
            seen = zone;
            continue;
        }
        if (seen != zone) {
            return true;
        }
    }
    return false;
}

void load_itemloc_item_in_multi_zone(const uint8_t* data, size_t size) {
    multi_zone_header = 0;
    multi_zone_words.clear();
    multi_zone_items.clear();
    if (!data || size < 8) {
        return;
    }
    BinReader in{data, size, 0, true};
    uint32_t version = in.u32();
    if (version - 1 > 4) {
        return;
    }
    multi_zone_header = static_cast<int32_t>(in.u32());
    uint8_t bitmap[0x2000];
    if (!in.take(bitmap, sizeof(bitmap))) {
        multi_zone_header = 0;
        return;
    }
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((bitmap[id >> 3] & (1 << (id & 7))) == 0) {
            continue;
        }
        multi_zone_skip_str(in);
        multi_zone_words[static_cast<uint32_t>(id)] = multi_zone_read_source(in, version);
    }
    uint32_t zones = in.u32();
    uint32_t running = 0;
    for (uint32_t i = 0; i < zones && in.ok; ++i) {
        running += multi_zone_varint(in);
        multi_zone_skip_str(in);
        uint16_t zone = in.u16();
        if (zone != 0xffff) {
            in.u8();
        }
        multi_zone_words[running | 0x01000000u] = zone;
    }
    uint32_t quests = in.u32();
    if (version >= 5) {
        in.u32();
    }
    running = 0;
    for (uint32_t i = 0; i < quests && in.ok; ++i) {
        running += multi_zone_varint(in);
        multi_zone_skip_str(in);
        uint16_t zone = in.u16();
        if (version >= 3) {
            uint8_t flags = in.u8();
            if (flags & 0x04) {
                multi_zone_skip_extra(in);
            }
        }
        multi_zone_words[running | 0x02000000u] = zone;
    }
    if (!in.take(bitmap, sizeof(bitmap))) {
        multi_zone_header = 0;
        multi_zone_words.clear();
        return;
    }
    multi_zone_items.assign(0x10000, {});
    size_t links = 0;
    for (int item = 0; item < 0x10000 && in.ok; ++item) {
        if ((bitmap[item >> 3] & (1 << (item & 7))) == 0) {
            continue;
        }
        while (in.ok) {
            uint8_t kind = in.u8();
            if (!in.ok || kind == 0xff) {
                break;
            }
            uint32_t count = multi_zone_varint(in);
            bool extra = kind < 22 && multi_zone_extra[kind];
            running = 0;
            for (uint32_t n = 0; n < count && in.ok; ++n) {
                running += multi_zone_varint(in);
                if (extra) {
                    in.u16();
                    in.u16();
                }
                uint16_t zone = 0;
                if (!multi_zone_zone_of(kind, running, &zone)) {
                    continue;
                }
                multi_zone_items[static_cast<size_t>(item)].push_back({zone, kind});
                ++links;
            }
        }
    }
    if (!in.ok) {
        multi_zone_header = 0;
        multi_zone_words.clear();
        multi_zone_items.clear();
        return;
    }
    log_line("ItemLoc multi-zone links %zu", links);
}

int native_itemloc_item_in_multi_zone(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int item = multi_zone_to_int(g_to_num(state, 1));
    bool mask = g_is_num(state, 2) && multi_zone_flag_on(g_to_num(state, 2));
    if (multi_zone_header <= 0) {
        return 0;
    }
    uint32_t id = static_cast<uint32_t>(item);
    if (id - 1u > 0xfffeu || multi_zone_items.size() <= id) {
        return 0;
    }
    const std::vector<MultiZoneLink>& links = multi_zone_items[id];
    if (links.empty() || !multi_zone_differs(links, mask)) {
        return 0;
    }
    g_push_num(state, 1.0);
    return 1;
}

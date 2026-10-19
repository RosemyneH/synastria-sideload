// ʕ •ᴥ•ʔ✿ ItemLocGetAllItemsInZone: eight optional numbers, one table of item ids. Counts are 7-bit groups, first byte most significant. ✿ ʕ •ᴥ•ʔ

struct AllItemsObj {
    uint32_t id = 0;
    uint16_t zone = 0xffff;
    uint8_t kind = 0;
    std::vector<uint16_t> down;
    std::vector<uint16_t> across;
};

struct AllItemsSrc {
    uint8_t type = 0;
    uint16_t obj = 0;
    uint16_t area = 0;
};

int all_items_gate = 0;
bool all_items_parsed = false;
std::vector<AllItemsObj> all_items_objs;
std::map<uint32_t, uint16_t> all_items_keys;
std::map<uint16_t, std::vector<AllItemsSrc>> all_items_lists;
std::map<uint16_t, std::vector<uint16_t>> all_items_zone_cache;
const uint8_t* all_items_areas = nullptr;
const uint8_t* all_items_maps = nullptr;
bool all_items_db_bound = false;

uint32_t all_items_msb(BinReader& in) {
    uint32_t value = 0;
    for (int i = 0; i < 5 && in.ok; ++i) {
        uint8_t byte = in.u8();
        value = (value << 7) | (byte & 0x7fu);
        if ((byte & 0x80u) == 0) {
            break;
        }
    }
    return value;
}

void all_items_cstr(BinReader& in) {
    while (in.ok && in.u8() != 0) {
    }
}

void all_items_skip_words(BinReader& in, std::vector<uint16_t>* out) {
    uint8_t count = in.u8();
    for (uint8_t i = 0; i < count && in.ok; ++i) {
        uint16_t word = in.u16();
        if (out) {
            out->push_back(word);
        }
    }
}

void all_items_bind_db() {
    if (all_items_db_bound) {
        return;
    }
    all_items_db_bound = true;
    void* found = nullptr;
    if (syn_resolve("g_areaTableDB", &found)) {
        all_items_areas = static_cast<const uint8_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("g_mapDB", &found)) {
        all_items_maps = static_cast<const uint8_t*>(found);
    }
}

const uint8_t* all_items_row(const uint8_t* table, int id) {
    if (!table || id < 0) {
        return nullptr;
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&max_id, table + 0x10, 4);
    memcpy(&min_id, table + 0x14, 4);
    if (id < min_id || id > max_id) {
        return nullptr;
    }
    void** by_id = nullptr;
    memcpy(&by_id, table + 0x30, sizeof(by_id));
    if (!by_id) {
        return nullptr;
    }
    return static_cast<const uint8_t*>(by_id[id - min_id]);
}

uint16_t all_items_put(uint32_t key, uint32_t id, uint8_t kind, uint16_t zone, std::vector<uint16_t> down, std::vector<uint16_t> across) {
    auto found = all_items_keys.find(key);
    if (found != all_items_keys.end()) {
        return found->second;
    }
    if (all_items_objs.size() >= 0x10000) {
        return 0;
    }
    AllItemsObj obj;
    obj.id = id;
    obj.zone = zone;
    obj.kind = kind;
    obj.down = std::move(down);
    obj.across = std::move(across);
    uint16_t slot = static_cast<uint16_t>(all_items_objs.size());
    all_items_objs.push_back(std::move(obj));
    all_items_keys.emplace(key, slot);
    return slot;
}

bool all_items_match(uint32_t query, uint32_t zone) {
    if (static_cast<uint16_t>(query) >= 0xfff0 || static_cast<uint16_t>(zone) >= 0xfff0) {
        return false;
    }
    if (static_cast<int16_t>(query) >= 0) {
        if (static_cast<int16_t>(zone) < 0) {
            return false;
        }
        return ((query ^ zone) & 0x3fffu) == 0;
    }
    if (static_cast<int16_t>(zone) < 0) {
        if (((query ^ zone) & 0x3ffu) != 0) {
            return false;
        }
        uint32_t diff = (query >> 10) & 0xfu;
        if (diff == 0) {
            return true;
        }
        return ((zone >> 10) & 0xfu) == diff;
    }
    const uint8_t* row = all_items_row(all_items_areas, static_cast<int>(zone & 0x3fffu));
    if (!row) {
        return false;
    }
    int32_t continent = 0;
    memcpy(&continent, row + 4, 4);
    if (continent != static_cast<int32_t>(query & 0x3ffu)) {
        return false;
    }
    return ((query >> 10) & 0xfu) == 0;
}

int all_items_walk(int id, int depth) {
    const uint8_t* row = all_items_row(all_items_areas, id);
    if (!row) {
        return 0;
    }
    for (;;) {
        int32_t parent = 0;
        memcpy(&parent, row + 8, 4);
        if (parent == 0) {
            return id;
        }
        if (depth >= 10) {
            return 0;
        }
        ++depth;
        id = parent;
        row = all_items_row(all_items_areas, id);
        if (!row) {
            return 0;
        }
    }
}

uint16_t all_items_type7(uint16_t id) {
    int area = id & 0x3fff;
    const uint8_t* row = all_items_row(all_items_areas, area);
    if (!row) {
        return 0xffff;
    }
    int32_t continent = 0;
    memcpy(&continent, row + 4, 4);
    if (continent != 0 && continent != 1 && continent != 0x212 && continent != 0x23b) {
        return static_cast<uint16_t>(static_cast<uint32_t>(continent) | 0x8000u);
    }
    int32_t parent = 0;
    memcpy(&parent, row + 8, 4);
    int zone = parent ? all_items_walk(parent, 1) : area;
    if (zone <= 0 || zone > 0xffff) {
        return 0xffff;
    }
    return static_cast<uint16_t>(zone);
}

bool all_items_contain(uint16_t id, uint32_t query, int depth) {
    if (depth >= 100) {
        return false;
    }
    auto found = all_items_keys.find(static_cast<uint32_t>(id) | 0x02000000u);
    if (found == all_items_keys.end()) {
        return false;
    }
    const AllItemsObj& obj = all_items_objs[found->second];
    if (depth > 0 && all_items_match(query, obj.zone)) {
        return true;
    }
    int next = depth + 1;
    for (uint16_t child : obj.down) {
        if (child == 0) {
            break;
        }
        if (all_items_contain(child, query, next)) {
            return true;
        }
    }
    for (uint16_t child : obj.across) {
        if (child == 0) {
            break;
        }
        if (all_items_contain(child, query, next)) {
            return true;
        }
    }
    return false;
}

bool all_items_in_zone(uint16_t item, uint32_t query) {
    auto found = all_items_lists.find(item);
    if (found == all_items_lists.end()) {
        return false;
    }
    for (const AllItemsSrc& src : found->second) {
        uint16_t zone = 0xffff;
        uint8_t kind = 0;
        uint32_t id = 0;
        if (src.obj == 0) {
            zone = all_items_type7(src.area);
            kind = 5;
            id = src.area;
        } else {
            const AllItemsObj& obj = all_items_objs[src.obj];
            zone = obj.zone;
            kind = obj.kind;
            id = obj.id;
        }
        if ((src.type == 0x14 || src.type == 0x15) && static_cast<int16_t>(zone) < 0 && zone < 0xfff0) {
            zone = static_cast<uint16_t>((zone & 0xe7ffu) | 0x2400u);
        }
        if (all_items_match(query, zone) || (kind == 2 && all_items_contain(static_cast<uint16_t>(id), query, 0))) {
            return true;
        }
    }
    return false;
}

void load_itemloc_all_items_in_zone(const uint8_t* data, size_t size) {
    all_items_gate = 0;
    all_items_parsed = true;
    all_items_objs.clear();
    all_items_keys.clear();
    all_items_lists.clear();
    all_items_zone_cache.clear();
    all_items_objs.emplace_back();
    all_items_bind_db();
    if (!data || size < 8u + 0x2000u) {
        return;
    }
    BinReader in{data, size, 0, true};
    uint32_t version = in.u32();
    uint32_t gate = in.u32();
    if (version < 1 || version > 5) {
        return;
    }
    uint8_t bits[0x2000];
    if (!in.take(bits, sizeof(bits))) {
        return;
    }
    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((bits[id >> 3] & (1 << (id & 7))) == 0) {
            continue;
        }
        all_items_cstr(in);
        uint16_t zone = in.u16();
        uint8_t spawn = zone == 0xffff ? 0 : in.u8();
        (void)spawn;
        if (version >= 2) {
            uint8_t mask = in.u8();
            for (int bit = 0; bit < 3; ++bit) {
                if (mask & (1 << bit)) {
                    in.u16();
                }
            }
        }
        all_items_put(static_cast<uint32_t>(id), static_cast<uint32_t>(id), 0, zone, {}, {});
    }
    uint32_t count = in.u32();
    uint32_t acc = 0;
    for (uint32_t n = 0; n < count && in.ok; ++n) {
        acc += all_items_msb(in);
        all_items_cstr(in);
        uint16_t zone = in.u16();
        if (zone != 0xffff) {
            in.u8();
        }
        all_items_put(acc | 0x01000000u, acc, 1, zone, {}, {});
    }
    count = in.u32();
    if (version >= 5) {
        in.u32();
    }
    acc = 0;
    for (uint32_t n = 0; n < count && in.ok; ++n) {
        acc += all_items_msb(in);
        all_items_cstr(in);
        uint16_t zone = in.u16();
        std::vector<uint16_t> down;
        std::vector<uint16_t> across;
        if (version >= 3) {
            uint8_t flags_byte = in.u8();
            if (flags_byte & 0x04) {
                uint32_t flags = in.u32();
                if (flags & 1u) {
                    in.u16();
                }
                if (flags & 8u) {
                    in.u16();
                }
                if (flags & 0x10u) {
                    in.u16();
                }
                if (flags & 0x20u) {
                    in.u8();
                }
                if (flags & 0x40u) {
                    in.u8();
                }
                if (flags & 0x80u) {
                    in.u16();
                    in.u16();
                }
                if (flags & 0x100u) {
                    in.u32();
                }
                if (flags & 0x200u) {
                    in.u32();
                }
                if (flags & 0x400u) {
                    in.u32();
                }
                if (flags & 0x800u) {
                    all_items_skip_words(in, nullptr);
                }
                if (flags & 0x1000u) {
                    in.u16();
                }
                if (flags & 0x2000u) {
                    all_items_skip_words(in, nullptr);
                }
                if (flags & 0x100000u) {
                    all_items_skip_words(in, nullptr);
                }
                if (flags & 0x200000u) {
                    all_items_skip_words(in, &down);
                }
                if (flags & 0x400000u) {
                    all_items_skip_words(in, &across);
                }
                if (flags & 0x800000u) {
                    all_items_skip_words(in, nullptr);
                }
            }
        }
        all_items_put(acc | 0x02000000u, acc, 2, zone, std::move(down), std::move(across));
    }
    if (!in.take(bits, sizeof(bits))) {
        all_items_objs.clear();
        all_items_keys.clear();
        return;
    }
    static const uint8_t k_case[] = {
        0, 1, 2, 3, 4, 0, 3, 5, 1, 0, 3, 3, 6, 0, 0, 7, 0, 1, 3, 8, 0, 1,
    };
    static const uint8_t k_extra[] = {
        1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
    };
    static const uint32_t k_prefix[] = {
        0, 0x01000000u, 0x02000000u, 0x03000000u, 0x04000000u, 0, 0x06000000u, 0, 0x15000000u,
    };
    static const uint8_t k_mint[] = {0, 0, 0, 3, 0, 0, 6, 0, 0x15};
    size_t sources = 0;
    for (int item = 0; item < 0x10000 && in.ok; ++item) {
        if ((bits[item >> 3] & (1 << (item & 7))) == 0) {
            continue;
        }
        std::vector<AllItemsSrc> list;
        while (in.ok) {
            uint8_t type = in.u8();
            if (!in.ok || type == 0xff) {
                break;
            }
            uint32_t group = all_items_msb(in);
            bool extra = type <= 0x15 && k_extra[type];
            acc = 0;
            uint8_t which = type <= 0x15 ? k_case[type] : 0;
            for (uint32_t n = 0; n < group && in.ok; ++n) {
                acc += all_items_msb(in);
                if (extra) {
                    in.u16();
                    in.u16();
                }
                if (which == 7) {
                    continue;
                }
                AllItemsSrc src;
                src.type = type;
                if (which == 5) {
                    src.area = static_cast<uint16_t>(acc);
                    list.push_back(src);
                    continue;
                }
                uint32_t key = acc | k_prefix[which];
                auto found = all_items_keys.find(key);
                uint16_t slot = 0;
                if (found != all_items_keys.end()) {
                    slot = found->second;
                } else if (which == 3 || which == 6 || which == 8) {
                    slot = all_items_put(key, acc, k_mint[which], 0xffff, {}, {});
                }
                if (slot == 0) {
                    continue;
                }
                src.obj = slot;
                list.push_back(src);
            }
        }
        if (!list.empty()) {
            sources += list.size();
            all_items_lists.emplace(static_cast<uint16_t>(item), std::move(list));
        }
    }
    size_t left = in.ok ? size - in.off : 0;
    if (!in.ok || left != 16) {
        all_items_objs.clear();
        all_items_keys.clear();
        all_items_lists.clear();
        log_line("ItemLoc all-items rejected at %zu left %zu", in.off, size - in.off);
        return;
    }
    all_items_gate = static_cast<int>(gate);
    log_line("ItemLoc all-items objs %zu items %zu sources %zu", all_items_objs.size() - 1, all_items_lists.size(), sources);
}

const std::vector<uint16_t>& all_items_zone_list(uint16_t zone) {
    auto found = all_items_zone_cache.find(zone);
    if (found != all_items_zone_cache.end()) {
        return found->second;
    }
    std::vector<uint16_t> list;
    uint32_t query = zone;
    for (const auto& entry : all_items_lists) {
        if (all_items_in_zone(entry.first, query)) {
            list.push_back(entry.first);
        }
    }
    auto placed = all_items_zone_cache.emplace(zone, std::move(list));
    return placed.first->second;
}

double all_items_floor(double n) {
    double whole = static_cast<double>(static_cast<int64_t>(n));
    if (n >= 0.0 || whole == n) {
        return whole;
    }
    return whole - 1.0;
}

int all_items_to_int(double n) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(n > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(n < k_max)) {
        return 0x7fffffff;
    }
    double floored = all_items_floor(n);
    if (!(floored > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(floored < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(floored);
}

bool all_items_read_int(void* state, int index, int* out) {
    if (!g_is_num(state, index)) {
        return false;
    }
    *out = all_items_to_int(g_to_num(state, index));
    return true;
}

bool all_items_listed(const int* ids, int count, int map) {
    for (int i = 0; i < count; ++i) {
        if (ids[i] == map) {
            return true;
        }
    }
    return false;
}

uint16_t all_items_here() {
    all_items_bind_db();
    static int32_t* map_id = nullptr;
    static int32_t* subzone = nullptr;
    static int32_t* area = nullptr;
    static bool bound = false;
    if (!bound) {
        bound = true;
        void* found = nullptr;
        if (syn_resolve("_ZN8CGGameUI15m_iCurrentMapIDE", &found)) {
            map_id = static_cast<int32_t*>(found);
        }
        found = nullptr;
        if (syn_resolve("_ZN8CGGameUI11m_subzoneIDE", &found)) {
            subzone = static_cast<int32_t*>(found);
        }
        found = nullptr;
        if (syn_resolve("_ZN8CGGameUI8m_areaIDE", &found)) {
            area = static_cast<int32_t*>(found);
        }
    }
    if (map_id && all_items_maps) {
        int map = *map_id;
        bool continent = map == 0 || map == 1 || map == 530 || map == 571;
        if (!continent && all_items_row(all_items_maps, map)) {
            uint16_t base = static_cast<uint16_t>(static_cast<uint32_t>(map) | 0x8000u);
            int diff = static_cast<int>(all_items_floor(game_num(12, 11)));
            if (diff == 10) {
                return static_cast<uint16_t>(base | 0x2400u);
            }
            static const int heroic[] = {
                269, 540, 542, 543, 545, 546, 547, 552, 553, 554, 555, 556, 557, 558, 560, 574, 575, 576,
                578, 585, 595, 599, 600, 601, 602, 604, 608, 619, 632, 650, 658, 668,
            };
            static const int sized[] = {249, 533, 603, 615, 616, 624};
            static const int flex[] = {631, 649, 724};
            if (all_items_listed(heroic, 32, map)) {
                return static_cast<uint16_t>(base | (diff == 0 ? 0x400u : 0x800u));
            }
            if (all_items_listed(sized, 6, map)) {
                return static_cast<uint16_t>(base | (diff == 0 ? 0x0c00u : 0x1000u));
            }
            if (all_items_listed(flex, 3, map) && diff >= 0 && diff < 4) {
                return static_cast<uint16_t>(base | static_cast<uint16_t>((diff + 5) << 10));
            }
            return base;
        }
    }
    if (!subzone || !area || !all_items_areas) {
        return 0;
    }
    const int ids[2] = {*subzone, *area};
    for (int id : ids) {
        const uint8_t* row = all_items_row(all_items_areas, id);
        if (!row) {
            continue;
        }
        int32_t parent = 0;
        memcpy(&parent, row + 8, 4);
        int root = parent == 0 ? id : all_items_walk(parent, 1);
        if (root > 0 && root <= 0xffff) {
            return static_cast<uint16_t>(root);
        }
    }
    return 0;
}

void all_items_face(int* klass, int* race) {
    *klass = 0;
    *race = 0;
    using ActiveFn = uint64_t (*)();
    using ObjectFn = void* (*)(uint64_t, uint32_t, const char*, int);
    static ActiveFn active = nullptr;
    static ObjectFn object = nullptr;
    static bool bound = false;
    if (!bound) {
        bound = true;
        void* found = nullptr;
        if (syn_resolve("_Z25ClntObjMgrGetActivePlayerv", &found)) {
            active = reinterpret_cast<ActiveFn>(found);
        }
        found = nullptr;
        if (syn_resolve("_Z19ClntObjMgrObjectPtr7WOWGUID8TypeMaskPKci", &found)) {
            object = reinterpret_cast<ObjectFn>(found);
        }
    }
    if (!active || !object) {
        return;
    }
    void* player = object(active(), 0x10, "test:1", 0xa0);
    if (!player) {
        return;
    }
    const uint8_t* unit = nullptr;
    memcpy(&unit, static_cast<uint8_t*>(player) + 0x160, sizeof(unit));
    if (!unit) {
        return;
    }
    *race = unit[0x44];
    *klass = unit[0x45];
    if (*klass > 0) {
        return;
    }
    for (int i = 0; i < g_class_count && i < 10; ++i) {
        if (g_classes[i].id >= 1 && g_classes[i].id <= 11) {
            *klass = g_classes[i].id;
            return;
        }
    }
}

bool all_items_armor(int cls, int level, int sub) {
    int primary = 1;
    int alt = 0;
    if (cls == 1 || cls == 2 || cls == 6) {
        primary = 4;
        if (level < 0x2d) {
            alt = 3;
        }
    } else if (cls == 4 || cls == 11) {
        primary = 2;
    } else if (cls == 3 || cls == 7) {
        primary = level < 0x2d ? 2 : 3;
    }
    if (primary == sub) {
        return true;
    }
    return alt != 0 && alt == sub;
}

int all_items_weapon(int cls, int inv, int klass, int sub) {
    if (inv == 0x16) {
        static const uint8_t block[] = {0, 1, 0, 0, 1, 0, 0};
        int index = cls - 1;
        if (index < 0 || index >= 7 || block[index]) {
            return -6;
        }
    } else if (inv == 0x0e) {
        if (cls != 1 && cls != 2 && cls != 7) {
            return -6;
        }
    }
    if (klass != 2) {
        return 0;
    }
    auto table = [&](const uint8_t* rows, int count) {
        int index = cls - 1;
        if (index < 0 || index >= count || rows[index]) {
            return -6;
        }
        return 0;
    };
    switch (sub) {
    case 0x13:
        return (cls == 5 || cls == 8 || cls == 9) ? 0 : -6;
    case 0x2:
    case 0x3:
    case 0x10:
    case 0x12:
        return (cls == 1 || cls == 3 || cls == 4) ? 0 : -6;
    case 6: {
        static const uint8_t rows[] = {0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0};
        return table(rows, 11);
    }
    case 5: {
        static const uint8_t rows[] = {0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0};
        return table(rows, 11);
    }
    case 4: {
        static const uint8_t rows[] = {0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0};
        return table(rows, 11);
    }
    case 0xf: {
        static const uint8_t rows[] = {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0};
        return table(rows, 11);
    }
    case 0xa: {
        static const uint8_t rows[] = {0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0};
        return table(rows, 11);
    }
    case 7: {
        static const uint8_t rows[] = {0, 0, 0, 0, 1, 0, 1, 0, 0};
        return table(rows, 9);
    }
    case 1: {
        static const uint8_t rows[] = {0, 0, 0, 1, 1, 0, 0};
        return table(rows, 7);
    }
    case 8: {
        static const uint8_t rows[] = {0, 0, 0, 1, 1, 0};
        return table(rows, 6);
    }
    case 0: {
        static const uint8_t rows[] = {0, 0, 0, 0, 1, 0, 0};
        return table(rows, 7);
    }
    case 0xd: {
        static const uint8_t rows[] = {0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0};
        return table(rows, 11);
    }
    default:
        return 0;
    }
}

int all_items_char_ok(const ItemRow& row, uint32_t bits, int race) {
    uint8_t tag = static_cast<uint8_t>(row.tag_word);
    if ((tag & 0x40) == 0) {
        return 0;
    }
    static const uint8_t inv_ok[] = {
        0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0,
    };
    int inv = row.inv;
    if (inv >= 0 && inv <= 0x1b && inv_ok[inv] == 0) {
        return 0;
    }
    if (row.flags & 0x01) {
        return 0;
    }
    if (row.flags & 0x08) {
        return 0;
    }
    if (bits == 0) {
        return 0;
    }
    if (inv != 0x10) {
        int sub = row.sub;
        bool hit = false;
        if (row.klass == 4 && sub >= 1 && sub <= 4) {
            int level = static_cast<int>(row.level & 0xffffu);
            for (int cls = 1; cls <= 11; ++cls) {
                if ((bits & (1u << (cls - 1))) == 0) {
                    continue;
                }
                if (all_items_armor(cls, level, sub)) {
                    hit = true;
                    break;
                }
            }
        } else {
            for (int cls = 1; cls <= 11; ++cls) {
                if ((bits & (1u << (cls - 1))) == 0) {
                    continue;
                }
                if (all_items_weapon(cls, inv, row.klass, sub) >= 0) {
                    hit = true;
                    break;
                }
            }
        }
        if (!hit) {
            return 0;
        }
    }
    uint32_t class_mask = row.class_mask & 0x5ffu;
    if (class_mask != 0 && class_mask != 0x5ffu && (bits & class_mask) == 0) {
        return 0;
    }
    uint32_t race_mask = row.race_mask & 0x6ffu;
    if (race_mask != 0 && race_mask != 0x6ffu) {
        uint32_t bit = race > 0 ? (1u << (race - 1)) : 0;
        if ((race_mask & bit) == 0) {
            return 0;
        }
    }
    return 1;
}

bool all_items_attuned(int item, int forge, uint32_t tag) {
    if ((tag & 0x40u) == 0 || item <= 0 || item > 0xffff) {
        return false;
    }
    for (int level = 0; level <= 3; ++level) {
        if (forge >= 0 && forge != level) {
            continue;
        }
        int key = item | (level << 24);
        if (game_num(11, key) >= 100.0) {
            return true;
        }
        if (tag & 0x1000u) {
            for (int cat : {16, 17, 20}) {
                if (static_cast<int>(game_num(cat, key) + 0.01) != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

int all_items_have(int item) {
    double cached = game_num(13, item);
    int count = 0;
    if (cached > 0.0) {
        count += static_cast<int>(all_items_floor(cached));
    }
    for (int slot = 0; slot < 0x13; ++slot) {
        void* held = item_count_at(0xff, slot);
        if (!held) {
            continue;
        }
        void* fields = nullptr;
        memcpy(&fields, static_cast<char*>(held) + 0x168, sizeof(fields));
        if (!fields) {
            continue;
        }
        int32_t entry = 0;
        memcpy(&entry, static_cast<char*>(fields) + 0xc, 4);
        if (entry == item) {
            int32_t stack = item_stack_count(held);
            if (stack > 0) {
                count += stack;
            }
        }
    }
    return count;
}

bool all_items_only_quests(uint16_t item) {
    auto found = all_items_lists.find(item);
    if (found == all_items_lists.end() || found->second.empty()) {
        return false;
    }
    for (const AllItemsSrc& src : found->second) {
        if (src.type != 2) {
            return false;
        }
    }
    return true;
}

using AllItemsTableFn = void (*)(void*, int, int);
using AllItemsSetFn = void (*)(void*, int);
using AllItemsCheckFn = int (*)(void*, int);

AllItemsTableFn all_items_table = nullptr;
AllItemsSetFn all_items_set = nullptr;
AllItemsCheckFn all_items_check = nullptr;
bool all_items_lua = false;

void all_items_bind_lua() {
    if (all_items_lua) {
        return;
    }
    all_items_lua = true;
    syn_resolve("lua_createtable", reinterpret_cast<void**>(&all_items_table));
    syn_resolve("lua_settable", reinterpret_cast<void**>(&all_items_set));
    syn_resolve("lua_checkstack", reinterpret_cast<void**>(&all_items_check));
}

void all_items_ensure() {
    if (all_items_parsed || !g_root[0]) {
        return;
    }
    char path[1024];
    snprintf(path, sizeof(path), "%s/WoWExtItemLoc.bin", g_root);
    size_t size = 0;
    uint8_t* bytes = read_alloc(path, &size);
    if (!bytes) {
        all_items_gate = -1;
        return;
    }
    load_itemloc_all_items_in_zone(bytes, size);
    free(bytes);
}

int native_itemloc_all_items_in_zone(void* state) {
    all_items_ensure();
    all_items_bind_lua();
    if (!g_is_num || !g_to_num || !g_push_num || !all_items_table || !all_items_set) {
        return 0;
    }
    all_items_table(state, 0, 0);
    int zone = -1;
    int must_char = 0;
    int must_acc = 0;
    int must_any = 1;
    int must_valid = 1;
    int must_have = 0;
    int forge = -1;
    int include_boe = 0;
    all_items_read_int(state, 1, &zone);
    all_items_read_int(state, 2, &must_char);
    all_items_read_int(state, 3, &must_acc);
    all_items_read_int(state, 4, &must_any);
    all_items_read_int(state, 5, &must_valid);
    all_items_read_int(state, 6, &must_have);
    all_items_read_int(state, 7, &forge);
    all_items_read_int(state, 8, &include_boe);
    if (zone < 0) {
        zone = all_items_here();
    }
    if (all_items_gate <= 0 || zone <= 0 || zone >= 0x10000) {
        return 1;
    }
    int klass = 0;
    int race = 0;
    uint32_t bits = 0;
    if (must_char != 0) {
        all_items_face(&klass, &race);
        if (klass >= 1 && klass <= 11) {
            bits |= 1u << (klass - 1);
        }
        int count = g_class_count < 10 ? g_class_count : 10;
        for (int i = 0; i < count; ++i) {
            int id = g_classes[i].id;
            if (id >= 1 && id <= 11) {
                bits |= 1u << (id - 1);
            }
        }
    }
    const std::vector<uint16_t>& items = all_items_zone_list(static_cast<uint16_t>(zone));
    int index = 1;
    for (uint16_t item : items) {
        const ItemRow* row = item_row(item);
        if (!row) {
            continue;
        }
        uint32_t tag = row->tag_word;
        uint8_t low = static_cast<uint8_t>(tag);
        uint8_t high = static_cast<uint8_t>(tag >> 8);
        if (must_valid != 0) {
            int bit = (low & 0x20) ? 0 : 1;
            if (bit != (must_valid > 0)) {
                continue;
            }
        }
        if (must_any != 0) {
            int bit = (low & 0x40) ? 1 : 0;
            if (bit != (must_any > 0)) {
                continue;
            }
        }
        if (must_char != 0) {
            bool keep = all_items_char_ok(*row, bits, race) > 0;
            if (!keep && include_boe > 0 && (high & 0x80) == 0 && (low & 0x40)) {
                keep = true;
            }
            if (keep && all_items_attuned(item, forge, tag)) {
                keep = false;
            }
            if (keep != (must_char > 0)) {
                continue;
            }
        }
        if (must_acc != 0) {
            bool keep = (low & 0x40) && !all_items_attuned(item, forge, tag);
            if (keep != (must_acc > 0)) {
                continue;
            }
        }
        if (forge > 0) {
            uint8_t bond = static_cast<uint8_t>(row->quality - 2);
            if ((low & 0x40) == 0 || bond > 2 || all_items_only_quests(item)) {
                continue;
            }
        }
        if (must_have != 0) {
            int count = all_items_have(item);
            if (must_have > 0 && count == 0) {
                continue;
            }
            if (must_have < 0 && count > 0) {
                continue;
            }
        }
        if (all_items_check) {
            all_items_check(state, 2);
        }
        g_push_num(state, index);
        g_push_num(state, item);
        all_items_set(state, -3);
        ++index;
    }
    return 1;
}

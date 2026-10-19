// ʕ •ᴥ•ʔ✿ ItemLocGetAllObjsInZone: optional zone and category, one table. Counts are MSB-first 7-bit groups, five bytes max. ✿ ʕ •ᴥ•ʔ

struct all_objs_row {
    uint32_t key = 0;
    uint16_t zone = 0;
};

struct all_objs_node {
    int prev = 0;
    int next = 0;
    uint32_t key = 0;
    uint16_t zone = 0;
};

std::vector<all_objs_row> all_objs_rows;
bool all_objs_ready = false;

using AllObjsTableFn = void (*)(void*, int, int);
using AllObjsSetFn = void (*)(void*, int);
using AllObjsCheckFn = int (*)(void*, int);

AllObjsTableFn all_objs_create = nullptr;
AllObjsSetFn all_objs_set = nullptr;
AllObjsCheckFn all_objs_check = nullptr;
bool all_objs_lua = false;

const uint8_t* all_objs_areas = nullptr;
const uint8_t* all_objs_maps = nullptr;
int32_t* all_objs_map_id = nullptr;
int32_t* all_objs_subzone = nullptr;
int32_t* all_objs_area = nullptr;
bool all_objs_bound = false;

uint32_t all_objs_var(BinReader& in) {
    uint8_t byte = in.u8();
    uint32_t value = byte & 0x7fu;
    for (int i = 0; i < 4 && (byte & 0x80u) != 0; ++i) {
        byte = in.u8();
        value = (value << 7) | (byte & 0x7fu);
    }
    return value;
}

void all_objs_cstr(BinReader& in) {
    while (in.ok && in.u8() != 0) {
    }
}

void all_objs_words(BinReader& in) {
    uint8_t count = in.u8();
    if (count) {
        in.off += static_cast<size_t>(count) * 2u;
        if (in.off > in.size) {
            in.ok = false;
            in.off = in.size;
        }
    }
}

void all_objs_tail(BinReader& in) {
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
        all_objs_words(in);
    }
    if (flags & 0x1000u) {
        in.u16();
    }
    if (flags & 0x2000u) {
        all_objs_words(in);
    }
    if (flags & 0x100000u) {
        all_objs_words(in);
    }
    if (flags & 0x200000u) {
        all_objs_words(in);
    }
    if (flags & 0x400000u) {
        all_objs_words(in);
    }
    if (flags & 0x800000u) {
        all_objs_words(in);
    }
}

uint32_t all_objs_hash(uint32_t key) {
    uint32_t value = 0x811c9dc5u;
    for (int i = 0; i < 4; ++i) {
        value ^= (key >> (8 * i)) & 0xffu;
        value *= 0x01000193u;
    }
    return value;
}

void all_objs_unlink(std::vector<all_objs_node>& nodes, int cur) {
    int prev = nodes[static_cast<size_t>(cur)].prev;
    int next = nodes[static_cast<size_t>(cur)].next;
    nodes[static_cast<size_t>(prev)].next = next;
    nodes[static_cast<size_t>(next)].prev = prev;
}

void all_objs_before(std::vector<all_objs_node>& nodes, int cur, int pos) {
    all_objs_unlink(nodes, cur);
    int prev = nodes[static_cast<size_t>(pos)].prev;
    nodes[static_cast<size_t>(cur)].prev = prev;
    nodes[static_cast<size_t>(cur)].next = pos;
    nodes[static_cast<size_t>(prev)].next = cur;
    nodes[static_cast<size_t>(pos)].prev = cur;
}

void all_objs_rehash(std::vector<all_objs_node>& nodes, std::vector<int>& bucket, int& mask, int& slots, int request) {
    unsigned top = static_cast<unsigned>(request - 1) | 1u;
    int bits = 31 - __builtin_clz(top);
    slots = 1 << (bits + 1);
    mask = slots - 1;
    bucket.assign(static_cast<size_t>(slots) * 2u, 0);
    int cur = nodes[0].next;
    while (cur != 0) {
        int next = nodes[static_cast<size_t>(cur)].next;
        int slot = static_cast<int>(all_objs_hash(nodes[static_cast<size_t>(cur)].key) & static_cast<uint32_t>(mask));
        int first = bucket[static_cast<size_t>(slot * 2)];
        if (first == 0) {
            bucket[static_cast<size_t>(slot * 2)] = cur;
            bucket[static_cast<size_t>(slot * 2 + 1)] = cur;
        } else {
            all_objs_before(nodes, cur, first);
            bucket[static_cast<size_t>(slot * 2)] = cur;
        }
        cur = next;
    }
}

void all_objs_insert(std::vector<all_objs_node>& nodes, std::vector<int>& bucket, int& mask, int& slots, int& count, uint32_t key, uint16_t zone) {
    auto place = [&]() -> int {
        int slot = static_cast<int>(all_objs_hash(key) & static_cast<uint32_t>(mask));
        int last = bucket[static_cast<size_t>(slot * 2 + 1)];
        if (last == 0) {
            return 0;
        }
        int head = bucket[static_cast<size_t>(slot * 2)];
        int at = last;
        for (;;) {
            if (nodes[static_cast<size_t>(at)].key == key) {
                return -1;
            }
            if (at == head) {
                return at;
            }
            at = nodes[static_cast<size_t>(at)].prev;
        }
    };
    if (static_cast<float>(count + 1) / static_cast<float>(slots) > 1.0f) {
        int need = count + 1;
        int next = slots;
        if (next < need) {
            if (next < 0x200) {
                next *= 8;
            }
            if (next < need) {
                next = need;
            }
        }
        all_objs_rehash(nodes, bucket, mask, slots, next);
    }
    int pos = place();
    if (pos < 0) {
        return;
    }
    int cur = static_cast<int>(nodes.size());
    all_objs_node node;
    node.key = key;
    node.zone = zone;
    nodes.push_back(node);
    int prev = nodes[static_cast<size_t>(pos)].prev;
    nodes[static_cast<size_t>(cur)].prev = prev;
    nodes[static_cast<size_t>(cur)].next = pos;
    nodes[static_cast<size_t>(prev)].next = cur;
    nodes[static_cast<size_t>(pos)].prev = cur;
    ++count;
    int slot = static_cast<int>(all_objs_hash(key) & static_cast<uint32_t>(mask));
    int first = bucket[static_cast<size_t>(slot * 2)];
    if (first == 0) {
        bucket[static_cast<size_t>(slot * 2)] = cur;
        bucket[static_cast<size_t>(slot * 2 + 1)] = cur;
    } else if (first == pos) {
        bucket[static_cast<size_t>(slot * 2)] = cur;
    } else if (bucket[static_cast<size_t>(slot * 2 + 1)] == prev) {
        bucket[static_cast<size_t>(slot * 2 + 1)] = cur;
    }
}

// ʕ •ᴥ•ʔ✿ 0 reads the chance words on a drop row; anything above 21 does not ✿ ʕ •ᴥ•ʔ
const uint8_t all_objs_plain[] = {
    0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
};

bool all_objs_drops(BinReader& in) {
    if (!in.ok || in.off + 0x2000 > in.size) {
        in.ok = false;
        return false;
    }
    const uint8_t* bits = in.data + in.off;
    in.off += 0x2000;
    for (int item = 0; item < 0x10000 && in.ok; ++item) {
        if ((bits[item >> 3] & static_cast<uint8_t>(1u << (item & 7))) == 0) {
            continue;
        }
        while (in.ok) {
            uint8_t src = in.u8();
            if (!in.ok || src == 0xff) {
                break;
            }
            uint32_t group = all_objs_var(in);
            bool plain = src > 21 || all_objs_plain[src] != 0;
            for (uint32_t n = 0; n < group && in.ok; ++n) {
                all_objs_var(in);
                if (!plain) {
                    in.u16();
                    in.u16();
                }
            }
        }
    }
    return in.ok && in.off + 16 == in.size;
}

void load_itemloc_all_objs_in_zone(const uint8_t* data, size_t size) {
    all_objs_rows.clear();
    all_objs_ready = true;
    if (!data || size < 8u + 0x2000u) {
        return;
    }
    BinReader in{data, size, 0, true};
    uint32_t version = in.u32();
    in.u32();
    if (!in.ok || version < 1 || version > 5 || in.off + 0x2000 > in.size) {
        log_line("WoWExtItemLoc.bin all-objs version %u rejected", version);
        return;
    }
    const uint8_t* bits = in.data + in.off;
    in.off += 0x2000;

    std::vector<all_objs_node> nodes(1);
    std::vector<int> bucket(16, 0);
    int mask = 7;
    int slots = 8;
    int count = 0;
    nodes[0].next = 0;
    nodes[0].prev = 0;
    nodes.reserve(24000);

    auto add = [&](uint32_t key, uint16_t zone) {
        all_objs_insert(nodes, bucket, mask, slots, count, key, zone);
    };

    for (int id = 0; id < 0x10000 && in.ok; ++id) {
        if ((bits[id >> 3] & static_cast<uint8_t>(1u << (id & 7))) == 0) {
            continue;
        }
        all_objs_cstr(in);
        uint16_t zone = in.u16();
        if (zone != 0xffff) {
            in.u8();
        }
        if (version >= 2) {
            uint8_t mask_byte = in.u8();
            for (int bit = 0; bit < 3; ++bit) {
                if (mask_byte & static_cast<uint8_t>(1u << bit)) {
                    in.u16();
                }
            }
        }
        add(static_cast<uint32_t>(id), zone);
    }

    uint32_t n1 = in.u32();
    uint32_t acc = 0;
    for (uint32_t i = 0; i < n1 && in.ok; ++i) {
        acc += all_objs_var(in);
        all_objs_cstr(in);
        uint16_t zone = in.u16();
        if (zone != 0xffff) {
            in.u8();
        }
        add(acc | 0x01000000u, zone);
    }

    uint32_t n2 = in.u32();
    if (version >= 5) {
        in.u32();
    }
    acc = 0;
    for (uint32_t i = 0; i < n2 && in.ok; ++i) {
        acc += all_objs_var(in);
        all_objs_cstr(in);
        uint16_t zone = in.u16();
        if (version >= 3) {
            uint8_t flags = in.u8();
            if (flags & 4u) {
                all_objs_tail(in);
            }
        }
        add(acc | 0x02000000u, zone);
    }

    if (!in.ok || !all_objs_drops(in)) {
        log_line("WoWExtItemLoc.bin all-objs truncated at %zu", in.off);
        return;
    }

    all_objs_rows.reserve(static_cast<size_t>(count));
    for (int cur = nodes[0].next; cur != 0; cur = nodes[static_cast<size_t>(cur)].next) {
        all_objs_row row;
        row.key = nodes[static_cast<size_t>(cur)].key;
        row.zone = nodes[static_cast<size_t>(cur)].zone;
        all_objs_rows.push_back(row);
    }
    log_line("WoWExtItemLoc.bin all-objs %zu", all_objs_rows.size());
}

void all_objs_ensure() {
    if (all_objs_ready || !g_root[0]) {
        return;
    }
    char path[1024];
    snprintf(path, sizeof(path), "%s/WoWExtItemLoc.bin", g_root);
    size_t size = 0;
    uint8_t* bytes = read_alloc(path, &size);
    if (!bytes) {
        all_objs_ready = true;
        return;
    }
    load_itemloc_all_objs_in_zone(bytes, size);
    free(bytes);
}

int all_objs_to_int(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(value < k_max)) {
        return 0x7fffffff;
    }
    double rounded = value + (value >= 0.0 ? 0.5 : -0.5);
    if (!(rounded > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

void all_objs_bind_db() {
    if (all_objs_bound) {
        return;
    }
    all_objs_bound = true;
    void* found = nullptr;
    if (syn_resolve("g_areaTableDB", &found)) {
        all_objs_areas = static_cast<const uint8_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("g_mapDB", &found)) {
        all_objs_maps = static_cast<const uint8_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN8CGGameUI15m_iCurrentMapIDE", &found)) {
        all_objs_map_id = static_cast<int32_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN8CGGameUI11m_subzoneIDE", &found)) {
        all_objs_subzone = static_cast<int32_t*>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN8CGGameUI8m_areaIDE", &found)) {
        all_objs_area = static_cast<int32_t*>(found);
    }
}

const uint8_t* all_objs_record(const uint8_t* table, int id) {
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

int all_objs_climb(int id, int depth) {
    int32_t min_id = 0;
    memcpy(&min_id, all_objs_areas + 0x14, 4);
    if (id < min_id) {
        return 0;
    }
    for (;;) {
        const uint8_t* row = all_objs_record(all_objs_areas, id);
        if (!row) {
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
        ++depth;
        id = parent;
        if (id < min_id) {
            return 0;
        }
    }
}

int all_objs_area_root(int id) {
    const uint8_t* row = all_objs_record(all_objs_areas, id);
    if (!row) {
        return 0;
    }
    int32_t parent = 0;
    memcpy(&parent, row + 8, 4);
    if (parent == 0) {
        return id;
    }
    return all_objs_climb(parent, 1);
}

bool all_objs_listed(const int* ids, int count, int map) {
    for (int i = 0; i < count; ++i) {
        if (ids[i] == map) {
            return true;
        }
    }
    return false;
}

int all_objs_kind(int map) {
    static const int heroic[] = {
        269, 540, 542, 543, 545, 546, 547, 552, 553, 554, 555, 556, 557, 558, 560, 574, 575, 576,
        578, 585, 595, 599, 600, 601, 602, 604, 608, 619, 632, 650, 658, 668,
    };
    static const int sized[] = {249, 533, 603, 615, 616, 624};
    static const int flex[] = {631, 649, 724};
    if (all_objs_listed(heroic, 32, map)) {
        return 0;
    }
    if (all_objs_listed(sized, 6, map)) {
        return 1;
    }
    if (all_objs_listed(flex, 3, map)) {
        return 2;
    }
    return 3;
}

uint16_t all_objs_bits(uint16_t base, int map, int diff) {
    if (diff == 10) {
        return static_cast<uint16_t>(base | 0x2400);
    }
    int kind = all_objs_kind(map);
    if (kind == 0) {
        return static_cast<uint16_t>(base | (diff == 0 ? 0x400 : 0x800));
    }
    if (kind == 1) {
        return static_cast<uint16_t>(base | (diff == 0 ? 0x0c00 : 0x1000));
    }
    if (kind == 2 && diff >= 0 && diff < 4) {
        return static_cast<uint16_t>(base | ((diff + 5) << 10));
    }
    return base;
}

// ʕ •ᴥ•ʔ✿ cl was 0, so difficulty bits stay on. The higher of the two area globals is tried first. ✿ ʕ •ᴥ•ʔ
uint16_t all_objs_current() {
    all_objs_bind_db();
    if (all_objs_map_id && all_objs_maps) {
        int map = *all_objs_map_id;
        bool continent = map == 0 || map == 1 || map == 530 || map == 571;
        if (!continent && all_objs_record(all_objs_maps, map)) {
            uint16_t base = static_cast<uint16_t>(static_cast<uint32_t>(map) | 0x8000u);
            return all_objs_bits(base, map, static_cast<int>(game_num(12, 11)));
        }
    }
    if (!all_objs_areas) {
        return 0;
    }
    int32_t* order[2] = {};
    int count = 0;
    if (all_objs_subzone && all_objs_area) {
        if (reinterpret_cast<uintptr_t>(all_objs_subzone) > reinterpret_cast<uintptr_t>(all_objs_area)) {
            order[0] = all_objs_subzone;
            order[1] = all_objs_area;
        } else {
            order[0] = all_objs_area;
            order[1] = all_objs_subzone;
        }
        count = 2;
    } else if (all_objs_subzone) {
        order[0] = all_objs_subzone;
        count = 1;
    } else if (all_objs_area) {
        order[0] = all_objs_area;
        count = 1;
    }
    for (int i = 0; i < count; ++i) {
        int root = all_objs_area_root(*order[i]);
        if (root > 0) {
            return static_cast<uint16_t>(root);
        }
    }
    return 0;
}

bool all_objs_match(uint32_t left, uint32_t right) {
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
    all_objs_bind_db();
    const uint8_t* row = all_objs_record(all_objs_areas, static_cast<int>(right & 0x3fffu));
    if (!row) {
        return false;
    }
    int32_t continent = 0;
    memcpy(&continent, row + 4, 4);
    if (continent != static_cast<int32_t>(left & 0x3ffu)) {
        return false;
    }
    return ((left >> 10) & 0xfu) == 0;
}

void all_objs_bind_lua() {
    if (all_objs_lua) {
        return;
    }
    all_objs_lua = true;
    syn_resolve("lua_createtable", reinterpret_cast<void**>(&all_objs_create));
    syn_resolve("lua_settable", reinterpret_cast<void**>(&all_objs_set));
    syn_resolve("lua_checkstack", reinterpret_cast<void**>(&all_objs_check));
}

double all_objs_number(uint32_t value) {
    double number = static_cast<double>(static_cast<int32_t>(value));
    if (value & 0x80000000u) {
        number += 4294967296.0;
    }
    return number;
}

// ʕ •ᴥ•ʔ✿ table[n] is the list key: category in the high byte, id below it. Negative category keeps every row. ✿ ʕ •ᴥ•ʔ
int native_itemloc_all_objs_in_zone(void* state) {
    all_objs_ensure();
    all_objs_bind_lua();
    if (!all_objs_create || !all_objs_set || !g_push_num) {
        return 0;
    }
    all_objs_create(state, 0, 0);
    int zone = -1;
    int kind = -1;
    if (g_is_num && g_to_num && g_is_num(state, 1)) {
        zone = all_objs_to_int(g_to_num(state, 1));
    }
    if (g_is_num && g_to_num && g_is_num(state, 2)) {
        kind = all_objs_to_int(g_to_num(state, 2));
    }
    if (zone < 0) {
        zone = all_objs_current();
    }
    uint16_t query = static_cast<uint16_t>(zone);
    int index = 1;
    for (const all_objs_row& row : all_objs_rows) {
        if (kind >= 0 && static_cast<int>(row.key >> 24) != kind) {
            continue;
        }
        if (!all_objs_match(query, row.zone)) {
            continue;
        }
        if (all_objs_check) {
            all_objs_check(state, 2);
        }
        g_push_num(state, index);
        g_push_num(state, all_objs_number(row.key));
        all_objs_set(state, -3);
        ++index;
    }
    return 1;
}

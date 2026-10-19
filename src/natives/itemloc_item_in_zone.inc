// ʕ •ᴥ•ʔ✿ ItemLocItemIsInZone: two numbers, push 1 or nothing. Area rows are g_areaTableDB max +0x10, min +0x14, records +0x30. ✿ ʕ •ᴥ•ʔ

struct InZoneObj {
    uint32_t id = 0;
    uint16_t zone = 0xffff;
    uint8_t kind = 0;
    std::vector<uint16_t> down;
    std::vector<uint16_t> across;
};

struct InZoneSrc {
    uint16_t slot = 0;
    uint8_t type = 0;
    uint16_t area = 0;
};

struct InZoneRead {
    const uint8_t* p = nullptr;
    const uint8_t* end = nullptr;
    bool ok = true;

    uint8_t u8() {
        if (p >= end) {
            ok = false;
            return 0;
        }
        return *p++;
    }

    uint16_t u16() {
        if (p + 2 > end) {
            ok = false;
            p = end;
            return 0;
        }
        uint16_t v = 0;
        memcpy(&v, p, 2);
        p += 2;
        return v;
    }

    uint32_t u32() {
        if (p + 4 > end) {
            ok = false;
            p = end;
            return 0;
        }
        uint32_t v = 0;
        memcpy(&v, p, 4);
        p += 4;
        return v;
    }

    void bytes(size_t n) {
        if (p + n > end) {
            ok = false;
            p = end;
            return;
        }
        p += n;
    }

    void cstr() {
        while (ok) {
            if (u8() == 0) {
                return;
            }
        }
    }

    uint32_t varint() {
        uint8_t c = u8();
        uint32_t v = c & 0x7fu;
        if ((c & 0x80u) == 0) {
            return v;
        }
        c = u8();
        v = (v << 7) | (c & 0x7fu);
        if ((c & 0x80u) == 0) {
            return v;
        }
        c = u8();
        v = (v << 7) | (c & 0x7fu);
        if ((c & 0x80u) == 0) {
            return v;
        }
        c = u8();
        v = (v << 7) | (c & 0x7fu);
        if ((c & 0x80u) == 0) {
            return v;
        }
        c = u8();
        return (v << 7) | (c & 0x7fu);
    }
};

int in_zone_gate = 0;
std::vector<InZoneObj> in_zone_objs;
std::map<uint32_t, uint16_t> in_zone_keys;
std::vector<int> in_zone_item_of;
std::vector<std::vector<InZoneSrc>> in_zone_lists;
const uint8_t* in_zone_area = nullptr;
bool in_zone_area_bound = false;

bool in_zone_bit(const uint8_t* bits, int index) {
    return (bits[index >> 3] & static_cast<uint8_t>(1u << (index & 7))) != 0;
}

bool in_zone_extra_words(uint8_t type) {
    static const uint8_t extra[] = {
        1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
    };
    if (type > 21) {
        return false;
    }
    return extra[type] != 0;
}

uint32_t in_zone_mask(uint8_t type) {
    static const uint32_t mask[] = {
        0, 0x01000000u, 0x02000000u, 0x03000000u, 0x04000000u, 0, 0x03000000u, 0x05000000u,
        0x01000000u, 0, 0x03000000u, 0x03000000u, 0x06000000u, 0, 0, 0, 0, 0x01000000u,
        0x03000000u, 0x15000000u, 0, 0x01000000u,
    };
    if (type > 21 || type == 15) {
        return 0xffffffffu;
    }
    return mask[type];
}

void in_zone_skip_words(InZoneRead& in, std::vector<uint16_t>* out) {
    uint8_t count = in.u8();
    for (uint8_t i = 0; i < count; ++i) {
        uint16_t word = in.u16();
        if (out) {
            out->push_back(word);
        }
    }
}

void in_zone_read_extra(InZoneRead& in, std::vector<uint16_t>& down, std::vector<uint16_t>& across) {
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
        in_zone_skip_words(in, nullptr);
    }
    if (flags & 0x1000u) {
        in.u16();
    }
    if (flags & 0x2000u) {
        in_zone_skip_words(in, nullptr);
    }
    if (flags & 0x100000u) {
        in_zone_skip_words(in, nullptr);
    }
    if (flags & 0x200000u) {
        in_zone_skip_words(in, &down);
    }
    if (flags & 0x400000u) {
        in_zone_skip_words(in, &across);
    }
    if (flags & 0x800000u) {
        in_zone_skip_words(in, nullptr);
    }
}

void in_zone_add(uint32_t key, uint32_t id, uint8_t kind, uint16_t zone, std::vector<uint16_t> down, std::vector<uint16_t> across) {
    InZoneObj obj;
    obj.id = id;
    obj.zone = zone;
    obj.kind = kind;
    obj.down = std::move(down);
    obj.across = std::move(across);
    uint16_t slot = static_cast<uint16_t>(in_zone_objs.size());
    in_zone_objs.push_back(std::move(obj));
    in_zone_keys.emplace(key, slot);
}

void in_zone_bind() {
    if (in_zone_area_bound) {
        return;
    }
    in_zone_area_bound = true;
    void* found = nullptr;
    if (syn_resolve("g_areaTableDB", &found)) {
        in_zone_area = static_cast<const uint8_t*>(found);
    }
}

bool in_zone_row(int id, const uint8_t** row) {
    in_zone_bind();
    if (!in_zone_area || id < 0) {
        return false;
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&max_id, in_zone_area + 0x10, 4);
    memcpy(&min_id, in_zone_area + 0x14, 4);
    if (id < min_id || id > max_id) {
        return false;
    }
    void** by_id = nullptr;
    memcpy(&by_id, in_zone_area + 0x30, sizeof(by_id));
    if (!by_id) {
        return false;
    }
    const uint8_t* found = static_cast<const uint8_t*>(by_id[id - min_id]);
    if (!found) {
        return false;
    }
    *row = found;
    return true;
}

int in_zone_walk(int id, int depth) {
    const uint8_t* row = nullptr;
    if (!in_zone_row(id, &row)) {
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
        if (!in_zone_row(id, &row)) {
            return 0;
        }
    }
}

uint16_t in_zone_type7(uint16_t id) {
    int area = id & 0x3fff;
    const uint8_t* row = nullptr;
    if (!in_zone_row(area, &row)) {
        return 0xffff;
    }
    int32_t field = 0;
    memcpy(&field, row + 4, 4);
    if (field != 0 && field != 1 && field != 0x212 && field != 0x23b) {
        return static_cast<uint16_t>(field | 0x8000);
    }
    int32_t parent = 0;
    memcpy(&parent, row + 8, 4);
    int zone = parent ? in_zone_walk(parent, 1) : area;
    if (zone <= 0 || zone > 0xffff) {
        return 0xffff;
    }
    return static_cast<uint16_t>(zone);
}

bool in_zone_match(uint32_t query, uint32_t zone) {
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
    const uint8_t* row = nullptr;
    if (!in_zone_row(static_cast<int>(zone & 0x3fffu), &row)) {
        return false;
    }
    int32_t field = 0;
    memcpy(&field, row + 4, 4);
    if (field != static_cast<int32_t>(query & 0x3ffu)) {
        return false;
    }
    return ((query >> 10) & 0xfu) == 0;
}

bool in_zone_contain(uint16_t id, uint32_t query, int depth) {
    if (depth >= 100) {
        return false;
    }
    auto found = in_zone_keys.find(static_cast<uint32_t>(id) | 0x02000000u);
    if (found == in_zone_keys.end()) {
        return false;
    }
    const InZoneObj& obj = in_zone_objs[found->second];
    if (depth > 0 && in_zone_match(query, obj.zone)) {
        return true;
    }
    int next = depth + 1;
    for (uint16_t child : obj.down) {
        if (child == 0) {
            break;
        }
        if (in_zone_contain(child, query, next)) {
            return true;
        }
    }
    for (uint16_t child : obj.across) {
        if (child == 0) {
            break;
        }
        if (in_zone_contain(child, query, next)) {
            return true;
        }
    }
    return false;
}

int in_zone_to_int(double n) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(n > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(n < k_max)) {
        return 0x7fffffff;
    }
    double rounded = n + (n >= 0.0 ? 0.5 : -0.5);
    if (!(rounded > k_min)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

bool in_zone_alive(uint16_t zone, uint8_t kind, const std::vector<uint16_t>& down, const std::vector<uint16_t>& across) {
    if (zone < 0xfff0) {
        return true;
    }
    if (kind != 2) {
        return false;
    }
    for (uint16_t word : down) {
        if (word) {
            return true;
        }
    }
    for (uint16_t word : across) {
        if (word) {
            return true;
        }
    }
    return false;
}

void load_itemloc_item_in_zone(const uint8_t* data, size_t size) {
    in_zone_gate = 0;
    in_zone_objs.clear();
    in_zone_keys.clear();
    in_zone_item_of.clear();
    in_zone_lists.clear();
    in_zone_objs.emplace_back();
    if (!data || size < 8u + 0x2000u) {
        return;
    }
    InZoneRead in{data, data + size, true};
    uint32_t version = in.u32();
    uint32_t gate = in.u32();
    if (version < 1 || version > 5) {
        return;
    }
    uint8_t bits[0x2000];
    if (in.p + sizeof(bits) > in.end) {
        return;
    }
    memcpy(bits, in.p, sizeof(bits));
    in.p += sizeof(bits);
    for (int index = 0; index < 0x10000 && in.ok; ++index) {
        if (!in_zone_bit(bits, index)) {
            continue;
        }
        in.cstr();
        uint16_t zone = in.u16();
        if (zone != 0xffff) {
            in.u8();
        }
        if (version >= 2) {
            uint8_t mask = in.u8();
            for (int bit = 0; bit < 3; ++bit) {
                if (mask & static_cast<uint8_t>(1u << bit)) {
                    in.u16();
                }
            }
        }
        if (in_zone_objs.size() >= 0x10000) {
            in.ok = false;
            break;
        }
        in_zone_add(static_cast<uint32_t>(index), static_cast<uint32_t>(index), 0, zone, {}, {});
    }
    uint32_t count = in.u32();
    uint32_t acc = 0;
    for (uint32_t n = 0; n < count && in.ok; ++n) {
        acc += in.varint();
        in.cstr();
        uint16_t zone = in.u16();
        if (zone != 0xffff) {
            in.u8();
        }
        if (in_zone_objs.size() >= 0x10000) {
            in.ok = false;
            break;
        }
        in_zone_add(acc | 0x01000000u, acc, 1, zone, {}, {});
    }
    count = in.u32();
    if (version >= 5) {
        in.u32();
    }
    acc = 0;
    for (uint32_t n = 0; n < count && in.ok; ++n) {
        acc += in.varint();
        in.cstr();
        uint16_t zone = in.u16();
        std::vector<uint16_t> down;
        std::vector<uint16_t> across;
        if (version >= 3) {
            uint8_t flags = in.u8();
            if (flags & 4u) {
                in_zone_read_extra(in, down, across);
            }
        }
        if (in_zone_objs.size() >= 0x10000) {
            in.ok = false;
            break;
        }
        in_zone_add(acc | 0x02000000u, acc, 2, zone, std::move(down), std::move(across));
    }
    if (in.p + sizeof(bits) > in.end) {
        in.ok = false;
    }
    if (!in.ok) {
        in_zone_objs.clear();
        in_zone_keys.clear();
        return;
    }
    memcpy(bits, in.p, sizeof(bits));
    in.p += sizeof(bits);
    in_zone_item_of.assign(0x10000, -1);
    size_t sources = 0;
    for (int item = 0; item < 0x10000 && in.ok; ++item) {
        if (!in_zone_bit(bits, item)) {
            continue;
        }
        std::vector<InZoneSrc> list;
        for (;;) {
            uint8_t type = in.u8();
            if (!in.ok || type == 0xff) {
                break;
            }
            uint32_t group = in.varint();
            bool words = in_zone_extra_words(type);
            acc = 0;
            for (uint32_t n = 0; n < group && in.ok; ++n) {
                acc += in.varint();
                if (words) {
                    in.u16();
                    in.u16();
                }
                if (type == 7) {
                    InZoneSrc src;
                    src.type = 7;
                    src.area = static_cast<uint16_t>(acc);
                    list.push_back(src);
                    continue;
                }
                uint32_t mask = in_zone_mask(type);
                if (mask == 0xffffffffu) {
                    continue;
                }
                auto found = in_zone_keys.find(acc | mask);
                if (found == in_zone_keys.end()) {
                    continue;
                }
                const InZoneObj& obj = in_zone_objs[found->second];
                if (!in_zone_alive(obj.zone, obj.kind, obj.down, obj.across)) {
                    continue;
                }
                InZoneSrc src;
                src.slot = found->second;
                src.type = type;
                list.push_back(src);
            }
        }
        if (!list.empty()) {
            in_zone_item_of[static_cast<size_t>(item)] = static_cast<int>(in_zone_lists.size());
            sources += list.size();
            in_zone_lists.push_back(std::move(list));
        }
    }
    if (!in.ok) {
        in_zone_objs.clear();
        in_zone_keys.clear();
        in_zone_item_of.clear();
        in_zone_lists.clear();
        return;
    }
    in_zone_gate = static_cast<int>(gate);
    log_line("ItemLoc item-in-zone objects %zu items %zu sources %zu", in_zone_objs.size() - 1, in_zone_lists.size(), sources);
}

int native_itemloc_item_in_zone(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int item = in_zone_to_int(g_to_num(state, 1));
    int zone = in_zone_to_int(g_to_num(state, 2));
    if (item <= 0 || zone <= 0 || zone > 0xffff || item >= 0x10000 || in_zone_gate <= 0) {
        return 0;
    }
    int list = in_zone_item_of[static_cast<size_t>(item)];
    if (list < 0) {
        return 0;
    }
    uint32_t query = static_cast<uint32_t>(zone);
    for (const InZoneSrc& src : in_zone_lists[static_cast<size_t>(list)]) {
        uint16_t zone_word = 0xffff;
        uint8_t kind = 0;
        uint32_t id = 0;
        if (src.slot == 0) {
            zone_word = in_zone_type7(src.area);
            kind = 5;
            id = src.area;
        } else {
            const InZoneObj& obj = in_zone_objs[src.slot];
            zone_word = obj.zone;
            kind = obj.kind;
            id = obj.id;
        }
        if (src.type == 0x14 || src.type == 0x15) {
            if (static_cast<int16_t>(zone_word) < 0 && zone_word < 0xfff0) {
                zone_word = static_cast<uint16_t>((zone_word & 0xe7ff) | 0x2400);
            }
        }
        if (in_zone_match(query, zone_word) || (kind == 2 && in_zone_contain(static_cast<uint16_t>(id), query, 0))) {
            g_push_num(state, 1.0);
            return 1;
        }
    }
    return 0;
}

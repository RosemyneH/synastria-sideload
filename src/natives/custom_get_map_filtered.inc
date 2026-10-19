struct MapFiltEntry {
    uint32_t head = 0;
    uint16_t kind = 0;
};

struct MapFiltObj {
    uint32_t key = 0;
    uint16_t flags = 0;
    std::string name;
    std::vector<MapFiltEntry> entries;
};

struct MapFiltHit {
    uint32_t key = 0;
    const char* name = "";
};

struct MapVec {
    float x;
    float y;
    float z;
};

using MapWorldFn = bool (*)(const MapVec&, int, float*, float*, bool*, bool, int);
using MapTableFn = void (*)(void*, int, int);
using MapRawFn = void (*)(void*, int);
using MapCheckFn = int (*)(void*, int);

std::vector<MapFiltObj> map_filt_objs;
std::map<uint32_t, uint32_t> map_filt_by_key;
bool map_filt_ready = false;
bool map_filt_tried = false;
MapWorldFn map_filt_world = nullptr;
MapTableFn map_filt_table = nullptr;
MapRawFn map_filt_rawset = nullptr;
MapCheckFn map_filt_check = nullptr;
bool map_filt_lua = false;

struct MapFiltIn {
    const uint8_t* p = nullptr;
    const uint8_t* end = nullptr;
    bool ok = true;

    bool take(size_t n) {
        if (!ok || p + n > end) {
            ok = false;
            return false;
        }
        return true;
    }

    uint8_t u8() {
        if (!take(1)) {
            return 0;
        }
        return *p++;
    }

    uint16_t u16() {
        if (!take(2)) {
            return 0;
        }
        uint16_t value = 0;
        memcpy(&value, p, 2);
        p += 2;
        return value;
    }

    uint32_t u32() {
        if (!take(4)) {
            return 0;
        }
        uint32_t value = 0;
        memcpy(&value, p, 4);
        p += 4;
        return value;
    }

    int8_t i8() {
        return static_cast<int8_t>(u8());
    }

    uint32_t u24() {
        uint32_t value = 0;
        memcpy(&value, p, take(3) ? 3 : 0);
        if (ok) {
            p += 3;
        }
        return value & 0xffffffu;
    }
};

uint16_t map_filt_axis(MapFiltIn& in, uint16_t base, int step, int wide, int mode) {
    if (mode & step) {
        int8_t delta = in.i8();
        if (mode & wide) {
            base = static_cast<uint16_t>(base + (delta < 0 ? -0x80 : 0x80));
        }
        return static_cast<uint16_t>(base + delta);
    }
    return in.u16();
}

uint32_t map_filt_xy(MapFiltIn& in, int mode, const MapFiltEntry* prev) {
    if (mode == 0) {
        return in.u32();
    }
    uint16_t x = prev ? static_cast<uint16_t>(prev->head) : 0;
    uint16_t y = prev ? static_cast<uint16_t>(prev->head >> 16) : 0;
    x = map_filt_axis(in, x, 1, 4, mode);
    y = map_filt_axis(in, y, 2, 8, mode);
    return static_cast<uint32_t>(x) | (static_cast<uint32_t>(y) << 16);
}

void map_filt_subs(MapFiltIn& in, uint32_t count) {
    const MapFiltEntry* prev = nullptr;
    MapFiltEntry held;
    for (uint32_t i = 0; i < count && in.ok; ++i) {
        uint8_t mode = in.u8();
        held.head = map_filt_xy(in, mode >> 4, i == 0 ? nullptr : prev);
        held.kind = (mode & 1) ? in.u8() : (prev ? prev->kind : 0);
        prev = &held;
    }
}

bool map_filt_load() {
    if (map_filt_tried) {
        return map_filt_ready;
    }
    map_filt_tried = true;
    if (!g_root[0]) {
        return false;
    }
    char path[1200];
    snprintf(path, sizeof(path), "%s/WoWExtObjLoc.bin", g_root);
    size_t size = 0;
    uint8_t* bytes = read_alloc(path, &size);
    if (!bytes) {
        return false;
    }
    MapFiltIn in{bytes, bytes + size, true};
    uint32_t version = in.u32();
    in.u32();
    in.u32();
    in.u32();
    in.u32();
    uint32_t groups = in.u32();
    uint32_t blob_size = in.u32();
    if (!in.ok || version != 1 || !in.take(blob_size)) {
        free(bytes);
        return false;
    }
    const uint8_t* blob = in.p;
    in.p += blob_size;
    if (in.u32() != 0x681caab0u) {
        free(bytes);
        return false;
    }
    std::vector<std::string> names;
    for (uint32_t i = 0; i < blob_size; ++i) {
        if (blob[i] != 0) {
            continue;
        }
        const char* text = reinterpret_cast<const char*>(blob + i + 1);
        size_t length = 0;
        while (i + 1 + length < blob_size && text[length] != 0) {
            ++length;
        }
        names.emplace_back(text, length);
    }
    for (uint32_t group = 0; group < groups && in.ok; ++group) {
        uint32_t kind = in.u8();
        uint32_t count = in.u32();
        uint32_t acc = 0;
        for (uint32_t n = 0; n < count && in.ok; ++n) {
            uint8_t flag = in.u8();
            if (flag & 1) {
                acc += in.u8();
            } else if (flag & 2) {
                acc += in.u16();
            } else {
                acc = in.u24();
            }
            MapFiltObj obj;
            obj.key = (kind << 24) | (acc & 0xffffffu);
            obj.flags = (flag & 4) ? in.u16() : 0;
            uint32_t spawns = 0;
            if ((flag & 0x18) == 0x18) {
                spawns = 1;
            } else if (flag & 8) {
                spawns = in.u8();
            } else if (flag & 0x10) {
                spawns = in.u16();
            }
            uint32_t name_id = 0;
            if (flag & 0x20) {
                name_id = in.u16();
            } else if (flag & 0x40) {
                name_id = in.u24();
            }
            if (name_id >= names.size()) {
                in.ok = false;
                break;
            }
            obj.name = names[name_id];
            if (spawns >= 0x10000u) {
                in.ok = false;
                break;
            }
            const MapFiltEntry* prev = nullptr;
            obj.entries.reserve(spawns);
            for (uint32_t s = 0; s < spawns && in.ok; ++s) {
                uint8_t mode = in.u8();
                MapFiltEntry entry;
                entry.kind = (mode & 1) ? in.u16() : (prev ? prev->kind : 0);
                entry.head = map_filt_xy(in, mode >> 4, s == 0 ? nullptr : prev);
                uint32_t extra = 0;
                if ((mode & 6) == 6) {
                    extra = in.u24();
                } else if (mode & 2) {
                    extra = in.u8();
                } else if (mode & 4) {
                    extra = in.u16();
                }
                if (extra >= 0x10000u) {
                    in.ok = false;
                    break;
                }
                if (extra) {
                    map_filt_subs(in, extra);
                }
                obj.entries.push_back(entry);
                prev = &obj.entries.back();
            }
            if (!in.ok) {
                break;
            }
            map_filt_by_key[obj.key] = static_cast<uint32_t>(map_filt_objs.size());
            map_filt_objs.push_back(std::move(obj));
        }
    }
    uint32_t tail = in.u32();
    free(bytes);
    if (!in.ok || tail != 0x681caabdu || map_filt_objs.empty()) {
        map_filt_objs.clear();
        map_filt_by_key.clear();
        return false;
    }
    map_filt_ready = true;
    log_line("map filter objs %zu", map_filt_objs.size());
    return true;
}

const MapFiltObj* map_filt_find(uint32_t key) {
    auto found = map_filt_by_key.find(key);
    if (found == map_filt_by_key.end()) {
        return nullptr;
    }
    return &map_filt_objs[found->second];
}

void map_filt_bind_lua() {
    if (map_filt_lua) {
        return;
    }
    map_filt_lua = true;
    void* found = nullptr;
    if (syn_resolve("lua_createtable", &found)) {
        map_filt_table = reinterpret_cast<MapTableFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_rawset", &found)) {
        map_filt_rawset = reinterpret_cast<MapRawFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_checkstack", &found)) {
        map_filt_check = reinterpret_cast<MapCheckFn>(found);
    }
    found = nullptr;
    if (syn_resolve("_ZN10CGWorldMap16GetWorldPositionERK8C3VectoriPfS3_Pbbi", &found)) {
        map_filt_world = reinterpret_cast<MapWorldFn>(found);
    }
}

bool map_filt_on_map(const MapFiltEntry& entry) {
    if (!map_filt_world) {
        return false;
    }
    MapVec pos{
        static_cast<float>(static_cast<int16_t>(entry.head & 0xffffu)),
        static_cast<float>(static_cast<int16_t>(entry.head >> 16)),
        0.0f,
    };
    float map_x = 0.0f;
    float map_y = 0.0f;
    bool outland = false;
    if (!map_filt_world(pos, entry.kind, &map_x, &map_y, &outland, false, 0)) {
        return false;
    }
    return map_x != 0.0f || map_y != 0.0f;
}

const char* map_filt_istr(const char* hay, const char* needle) {
    if (!hay || !needle || !needle[0]) {
        return nullptr;
    }
    for (const char* at = hay; *at; ++at) {
        const char* left = at;
        const char* right = needle;
        while (*left && *right) {
            unsigned char a = static_cast<unsigned char>(*left);
            unsigned char b = static_cast<unsigned char>(*right);
            if (a >= 'A' && a <= 'Z') {
                a = static_cast<unsigned char>(a + 32);
            }
            if (b >= 'A' && b <= 'Z') {
                b = static_cast<unsigned char>(b + 32);
            }
            if (a != b) {
                break;
            }
            ++left;
            ++right;
        }
        if (*right == 0) {
            return at;
        }
    }
    return nullptr;
}

int map_filt_icmp(const char* left, const char* right) {
    if (!left) {
        left = "";
    }
    if (!right) {
        right = "";
    }
    while (*left && *right) {
        unsigned char a = static_cast<unsigned char>(*left);
        unsigned char b = static_cast<unsigned char>(*right);
        if (a >= 'A' && a <= 'Z') {
            a = static_cast<unsigned char>(a + 32);
        }
        if (b >= 'A' && b <= 'Z') {
            b = static_cast<unsigned char>(b + 32);
        }
        if (a != b) {
            return a < b ? -1 : 1;
        }
        ++left;
        ++right;
    }
    if (*left == *right) {
        return 0;
    }
    return *left ? 1 : -1;
}

bool map_filt_item_ok(uint32_t id, std::map<uint32_t, uint8_t>& seen);

bool map_filt_obj_ok(uint32_t key, std::map<uint32_t, uint8_t>& seen) {
    const MapFiltObj* obj = map_filt_find(key);
    if (!obj || obj->entries.empty()) {
        return false;
    }
    auto found = seen.find(key);
    if (found != seen.end()) {
        return found->second != 0;
    }
    seen[key] = 0;
    if (obj->flags & 1) {
        for (const MapFiltEntry& entry : obj->entries) {
            uint32_t child = entry.head;
            bool hit = false;
            if (entry.kind == 1) {
                hit = map_filt_find(child) && map_filt_obj_ok(child, seen);
            } else if (entry.kind == 2) {
                child |= 0x01000000u;
                hit = map_filt_find(child) && map_filt_obj_ok(child, seen);
            } else if (entry.kind == 3) {
                hit = map_filt_item_ok(child & 0xffffu, seen);
            }
            if (hit) {
                seen[key] = 1;
                return true;
            }
        }
        return false;
    }
    for (const MapFiltEntry& entry : obj->entries) {
        if (map_filt_on_map(entry)) {
            seen[key] = 1;
            return true;
        }
    }
    return false;
}

bool map_filt_item_ok(uint32_t id, std::map<uint32_t, uint8_t>& seen) {
    if (id >= 0x10000u || !src_at_ready || id >= src_at_items.size() || src_at_items[id].empty()) {
        return false;
    }
    uint32_t key = id | 0x03000000u;
    auto found = seen.find(key);
    if (found != seen.end()) {
        return found->second != 0;
    }
    seen[key] = 0;
    static const uint8_t skip[] = {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0};
    for (const SrcAtSrc& src : src_at_items[id]) {
        uint8_t type = src.type;
        if (type == 0x14) {
            type = 0;
        } else if (type == 0x15) {
            type = 1;
        }
        if (type > 0x0e || skip[type]) {
            continue;
        }
        if (src.slot == 0 || src.slot >= src_at_objs.size()) {
            continue;
        }
        const SrcAtObj& row = src_at_objs[src.slot];
        if (row.type > 3) {
            continue;
        }
        uint32_t obj_key = (static_cast<uint32_t>(row.type) << 24) | row.id;
        const MapFiltObj* obj = map_filt_find(obj_key);
        if (!obj) {
            continue;
        }
        if (map_filt_obj_ok(obj_key, seen)) {
            seen[key] = 1;
            return true;
        }
    }
    return false;
}

int map_filt_item_top() {
    int id = 0xffff;
    int limit = static_cast<int>(g_items.size());
    while (id > 0) {
        if (id < limit && g_items[static_cast<size_t>(id)].on) {
            return id;
        }
        --id;
    }
    return 0;
}

double map_filt_u2d(uint32_t key) {
    double value = static_cast<double>(static_cast<int32_t>(key));
    if (key & 0x80000000u) {
        value += 4294967296.0;
    }
    return value;
}

// ʕ •ᴥ•ʔ✿ one search string. Empty or missing string returns nothing. Otherwise a 1-based table of at most 100 keys, names sorted with _stricmp. Keys already in the track set are left out. ✿ ʕ •ᴥ•ʔ
int native_get_map_filtered(void* state) {
    if (!g_is_str || !g_to_str || !g_is_str(state, 1)) {
        return 0;
    }
    const char* needle = g_to_str(state, 1, nullptr);
    if (!needle || needle[0] == 0) {
        return 0;
    }
    if (!map_filt_load()) {
        return 0;
    }
    map_filt_bind_lua();
    if (!map_filt_table || !map_filt_rawset || !g_push_num) {
        return 0;
    }
    std::map<uint32_t, uint8_t> seen;
    std::vector<MapFiltHit> hits;
    for (const MapFiltObj& obj : map_filt_objs) {
        if (obj.entries.empty() || !map_filt_istr(obj.name.c_str(), needle)) {
            continue;
        }
        if (!map_filt_obj_ok(obj.key, seen) || track_obj_ids.count(obj.key)) {
            continue;
        }
        hits.push_back({obj.key, obj.name.c_str()});
    }
    if (src_at_ready) {
        int top = map_filt_item_top();
        for (int id = 1; id < top; ++id) {
            if (static_cast<size_t>(id) >= g_items.size() || !g_items[static_cast<size_t>(id)].on) {
                continue;
            }
            const std::string& name = g_items[static_cast<size_t>(id)].name;
            if (!map_filt_istr(name.c_str(), needle)) {
                continue;
            }
            uint32_t key = static_cast<uint32_t>(id) | 0x03000000u;
            if (!map_filt_item_ok(static_cast<uint32_t>(id), seen) || track_obj_ids.count(key)) {
                continue;
            }
            hits.push_back({key, name.c_str()});
        }
    }
    if (hits.size() > 1) {
        std::sort(hits.begin(), hits.end(), [](const MapFiltHit& left, const MapFiltHit& right) {
            return map_filt_icmp(left.name, right.name) < 0;
        });
    }
    map_filt_table(state, 0, 0);
    int index = 1;
    for (const MapFiltHit& hit : hits) {
        if (index > 100) {
            break;
        }
        if (map_filt_check) {
            map_filt_check(state, 2);
        }
        g_push_num(state, index);
        g_push_num(state, map_filt_u2d(hit.key));
        map_filt_rawset(state, -3);
        ++index;
    }
    return 1;
}

// ʕ •ᴥ•ʔ✿ Custom_CalculateMapMarker fills spawn and waypoint lists. Callers read them back. ✿ ʕ •ᴥ•ʔ

struct MapMarkerRec {
    uint32_t key = 0;
    uint32_t parent = 0;
    float x = 0;
    float y = 0;
    uint8_t icon = 0;
    uint8_t flags = 0;
    uint16_t wpid = 0;
    uint16_t wpnum = 0;
};

struct MapMarkerPt {
    int16_t x = 0;
    int16_t y = 0;
    uint16_t zone = 0;
    uint16_t ways = 0;
    uint32_t way_at = 0;
    uint32_t raw = 0;
};

struct MapMarkerWay {
    int16_t x = 0;
    int16_t y = 0;
    uint8_t tag = 0;
};

struct MapMarkerObj {
    uint16_t mode = 0;
    uint32_t count = 0;
    uint32_t at = 0;
};

std::vector<MapMarkerRec> map_marker_spawns;
std::vector<MapMarkerRec> map_marker_waypoints;
int map_marker_total = 0;
int map_marker_limit = 1000;
float map_marker_cell = 1.f;

std::map<uint32_t, MapMarkerObj> map_marker_objs;
std::vector<MapMarkerPt> map_marker_pts;
std::vector<MapMarkerWay> map_marker_ways;
bool map_marker_objs_ready = false;

using MapMarkerWorldFn = bool (*)(const float*, int, float*, float*, bool*, bool, int);

MapMarkerWorldFn map_marker_world() {
    static MapMarkerWorldFn fn = nullptr;
    static bool bound = false;
    if (!bound) {
        bound = true;
        syn_resolve("_ZN10CGWorldMap16GetWorldPositionERK8C3VectoriPfS3_Pbbi", reinterpret_cast<void**>(&fn));
    }
    return fn;
}

struct MapMarkerIn {
    const uint8_t* p = nullptr;
    const uint8_t* end = nullptr;
    bool ok = true;

    bool take(void* dst, size_t n) {
        if (!ok || p + n > end) {
            ok = false;
            return false;
        }
        memcpy(dst, p, n);
        p += n;
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

    uint32_t u24() {
        uint8_t bytes[3] = {};
        take(bytes, 3);
        return bytes[0] | (uint32_t(bytes[1]) << 8) | (uint32_t(bytes[2]) << 16);
    }
};

int16_t map_marker_axis(MapMarkerIn& in, int bits, int bit, int wide, int prev) {
    if ((bits & bit) == 0) {
        return static_cast<int16_t>(in.u16());
    }
    int delta = static_cast<int8_t>(in.u8());
    int base = prev;
    if (bits & wide) {
        base += delta < 0 ? -0x80 : 0x80;
    }
    return static_cast<int16_t>(base + delta);
}

void map_marker_xy(MapMarkerIn& in, int bits, bool has_prev, int16_t px, int16_t py, int16_t* x, int16_t* y, uint32_t* raw) {
    if (bits == 0) {
        uint32_t word = in.u32();
        *raw = word;
        *x = static_cast<int16_t>(word & 0xffffu);
        *y = static_cast<int16_t>(word >> 16);
        return;
    }
    *x = map_marker_axis(in, bits, 1, 4, has_prev ? px : 0);
    *y = map_marker_axis(in, bits, 2, 8, has_prev ? py : 0);
    *raw = uint16_t(*x) | (uint32_t(uint16_t(*y)) << 16);
}

void map_marker_load_objs() {
    if (map_marker_objs_ready || !g_root[0]) {
        return;
    }
    map_marker_objs_ready = true;
    char path[1200];
    snprintf(path, sizeof(path), "%s/WoWExtObjLoc.bin", g_root);
    size_t size = 0;
    uint8_t* bytes = read_alloc(path, &size);
    if (!bytes) {
        log_line("map markers objloc missing");
        return;
    }
    MapMarkerIn in{bytes, bytes + size, true};
    uint32_t version = in.u32();
    in.u32();
    uint32_t obj_n = in.u32();
    uint32_t pt_n = in.u32();
    uint32_t way_n = in.u32();
    uint32_t groups = in.u32();
    uint32_t blob_n = in.u32();
    if (!in.ok || blob_n > static_cast<uint32_t>(in.end - in.p)) {
        free(bytes);
        log_line("map markers objloc short");
        return;
    }
    const uint8_t* blob = in.p;
    in.p += blob_n;
    uint32_t magic = in.u32();
    uint32_t strings = 0;
    for (uint32_t i = 0; i < blob_n; ++i) {
        if (blob[i] == 0) {
            ++strings;
        }
    }
    if (version != 1 || magic != 0x681caab0u || !in.ok) {
        free(bytes);
        log_line("map markers objloc header");
        return;
    }
    map_marker_pts.reserve(pt_n);
    map_marker_ways.reserve(way_n);
    uint16_t last_zone = 0;
    uint8_t last_tag = 0;
    bool failed = false;
    for (uint32_t g = 0; g < groups && !failed; ++g) {
        uint8_t kind = in.u8();
        uint32_t count = in.u32();
        uint32_t acc = 0;
        for (uint32_t n = 0; n < count && !failed; ++n) {
            uint8_t flags = in.u8();
            if (flags & 1u) {
                acc += in.u8();
            } else if (flags & 2u) {
                acc += in.u16();
            } else {
                acc = in.u24();
            }
            uint32_t key = (uint32_t(kind) << 24) | acc;
            uint16_t mode = (flags & 4u) ? in.u16() : 0;
            uint32_t points = 0;
            if ((flags & 0x18u) == 0x18u) {
                points = 1;
            } else if (flags & 8u) {
                points = in.u8();
            } else if (flags & 0x10u) {
                points = in.u16();
            }
            uint32_t name = 0;
            if (flags & 0x20u) {
                name = in.u16();
            }
            if (flags & 0x40u) {
                name = in.u24();
            }
            if (!in.ok || name >= strings || points >= 0x10000u || map_marker_pts.size() + points > pt_n) {
                failed = true;
                break;
            }
            MapMarkerObj obj;
            obj.mode = mode;
            obj.count = points;
            obj.at = static_cast<uint32_t>(map_marker_pts.size());
            map_marker_objs[key] = obj;
            int16_t px = 0;
            int16_t py = 0;
            bool has_prev = false;
            for (uint32_t p = 0; p < points; ++p) {
                uint8_t pf = in.u8();
                uint16_t zone = (pf & 1u) ? in.u16() : last_zone;
                last_zone = zone;
                MapMarkerPt pt;
                map_marker_xy(in, pf >> 4, has_prev, px, py, &pt.x, &pt.y, &pt.raw);
                px = pt.x;
                py = pt.y;
                has_prev = true;
                pt.zone = zone;
                uint32_t ways = 0;
                if ((pf & 6u) == 6u) {
                    ways = in.u24();
                } else if (pf & 2u) {
                    ways = in.u8();
                } else if (pf & 4u) {
                    ways = in.u16();
                }
                if (!in.ok || ways >= 0x10000u || map_marker_ways.size() + ways > way_n) {
                    failed = true;
                    break;
                }
                pt.ways = static_cast<uint16_t>(ways);
                pt.way_at = static_cast<uint32_t>(map_marker_ways.size());
                int16_t wx = 0;
                int16_t wy = 0;
                bool has_way = false;
                for (uint32_t w = 0; w < ways; ++w) {
                    uint8_t wf = in.u8();
                    uint8_t tag = (wf & 1u) ? in.u8() : last_tag;
                    last_tag = tag;
                    MapMarkerWay way;
                    uint32_t raw = 0;
                    map_marker_xy(in, wf >> 4, has_way, wx, wy, &way.x, &way.y, &raw);
                    wx = way.x;
                    wy = way.y;
                    has_way = true;
                    way.tag = tag;
                    map_marker_ways.push_back(way);
                }
                map_marker_pts.push_back(pt);
            }
        }
    }
    uint32_t end_magic = in.u32();
    free(bytes);
    if (failed || !in.ok || end_magic != 0x681caabdu || map_marker_pts.size() != pt_n || map_marker_ways.size() != way_n || map_marker_objs.size() != obj_n) {
        map_marker_objs.clear();
        map_marker_pts.clear();
        map_marker_ways.clear();
        log_line("map markers objloc rejected");
        return;
    }
    log_line("map markers objloc objs %zu points %zu ways %zu", map_marker_objs.size(), map_marker_pts.size(), map_marker_ways.size());
}

int map_marker_int(double value) {
    constexpr double lo = -2147483648.0;
    constexpr double hi = 2147483647.0;
    if (!(lo < value)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(value < hi)) {
        return 0x7fffffff;
    }
    double rounded = value + (value >= 0.0 ? 0.5 : -0.5);
    if (!(lo < rounded)) {
        return static_cast<int>(0x80000000u);
    }
    if (!(rounded < hi)) {
        return 0x7fffffff;
    }
    return static_cast<int>(rounded);
}

bool map_marker_source_ok(uint8_t type) {
    uint8_t index = type;
    if (type == 0x14) {
        index = 0;
    } else if (type == 0x15) {
        index = 1;
    }
    if (index > 0x0e) {
        return false;
    }
    static const uint8_t skip[] = {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0};
    return skip[index] == 0;
}

bool map_marker_project(int16_t x, int16_t y, uint16_t zone, float* out_x, float* out_y) {
    MapMarkerWorldFn world = map_marker_world();
    if (!world) {
        return false;
    }
    // ʕ •ᴥ•ʔ✿ Spawn yards go through CGWorldMap::GetWorldPosition on the map the client has open. ✿ ʕ •ᴥ•ʔ
    float pos[3] = {static_cast<float>(x), static_cast<float>(y), 0.f};
    float mx = 0;
    float my = 0;
    bool outland = false;
    if (!world(pos, zone, &mx, &my, &outland, false, 0)) {
        return false;
    }
    if (mx == 0.f && my == 0.f) {
        return false;
    }
    *out_x = mx;
    *out_y = my;
    return true;
}

bool map_marker_cell_new(float x, float y, std::map<uint32_t, uint8_t>& cells) {
    float cell = map_marker_cell < 0.01f ? 0.01f : map_marker_cell;
    int qx = static_cast<int>((x * 100.f) / cell);
    int qy = static_cast<int>((y * 100.f) / cell);
    uint32_t key = (uint32_t(qy) << 16) | uint32_t(qx);
    return cells.emplace(key, 0).second;
}

void map_marker_place(uint32_t key, uint32_t parent, int flags, uint32_t seen, std::map<uint32_t, uint8_t>& been, std::map<uint32_t, uint8_t>& cells);

void map_marker_sources(uint32_t key, uint32_t parent, int flags, uint32_t seen, std::map<uint32_t, uint8_t>& been, std::map<uint32_t, uint8_t>& cells) {
    uint32_t id = key & 0xffffffu;
    if (id >= 0x10000u || !all_items_parsed) {
        return;
    }
    auto list = all_items_lists.find(static_cast<uint16_t>(id));
    if (list == all_items_lists.end()) {
        return;
    }
    static const uint8_t which_of[] = {
        0, 1, 2, 3, 4, 0, 3, 5, 1, 0, 3, 3, 6, 0, 0, 7, 0, 1, 3, 8, 0, 1,
    };
    static const uint32_t prefix_of[] = {
        0, 0x01000000u, 0x02000000u, 0x03000000u, 0x04000000u, 0, 0x06000000u, 0, 0x15000000u,
    };
    uint32_t hold = parent ? parent : key;
    for (const AllItemsSrc& src : list->second) {
        if (!map_marker_source_ok(src.type) || src.type > 0x15 || src.obj == 0 || src.obj >= all_items_objs.size()) {
            continue;
        }
        uint8_t which = which_of[src.type];
        if (which > 8) {
            continue;
        }
        uint32_t prefix = prefix_of[which];
        if ((prefix >> 24) > 3u) {
            continue;
        }
        uint32_t child = all_items_objs[src.obj].id | prefix;
        map_marker_place(child, hold, flags, seen, been, cells);
    }
}

void map_marker_place(uint32_t key, uint32_t parent, int flags, uint32_t seen, std::map<uint32_t, uint8_t>& been, std::map<uint32_t, uint8_t>& cells) {
    if (!been.emplace(key, 0).second) {
        return;
    }
    uint32_t type = key >> 24;
    if (type < 8u) {
        seen |= 1u << type;
    }
    if (type == 3u) {
        map_marker_sources(key, parent, flags, seen, been, cells);
        return;
    }
    auto found = map_marker_objs.find(key);
    if (found == map_marker_objs.end()) {
        return;
    }
    const MapMarkerObj& obj = found->second;
    if (obj.at + obj.count > map_marker_pts.size()) {
        return;
    }
    uint32_t hold = parent ? parent : key;
    if (obj.mode & 1u) {
        for (uint32_t i = 0; i < obj.count; ++i) {
            const MapMarkerPt& pt = map_marker_pts[obj.at + i];
            uint32_t child = pt.raw;
            if (pt.zone == 1) {
            } else if (pt.zone == 2) {
                child |= 0x01000000u;
            } else if (pt.zone == 3) {
                child |= 0x03000000u;
            } else {
                continue;
            }
            map_marker_place(child, hold, flags, seen, been, cells);
        }
        return;
    }
    uint8_t mask = static_cast<uint8_t>(seen);
    for (uint32_t i = 0; i < obj.count; ++i) {
        const MapMarkerPt& pt = map_marker_pts[obj.at + i];
        float mx = 0;
        float my = 0;
        if (!map_marker_project(pt.x, pt.y, pt.zone, &mx, &my) || !map_marker_cell_new(mx, my, cells)) {
            continue;
        }
        MapMarkerRec rec;
        rec.key = key;
        rec.parent = parent;
        rec.x = mx;
        rec.y = my;
        rec.flags = mask;
        map_marker_spawns.push_back(rec);
        if (flags & 1) {
            continue;
        }
        uint16_t group = 0;
        uint16_t number = 0;
        if (pt.way_at + pt.ways > map_marker_ways.size()) {
            continue;
        }
        for (uint16_t w = 0; w < pt.ways; ++w) {
            const MapMarkerWay& way = map_marker_ways[pt.way_at + w];
            if (!map_marker_project(way.x, way.y, pt.zone, &mx, &my) || !map_marker_cell_new(mx, my, cells)) {
                continue;
            }
            uint16_t id = static_cast<uint16_t>(way.tag + 1u);
            uint16_t seq = group == id ? number : 0;
            seq = static_cast<uint16_t>(seq + 1u);
            group = id;
            number = seq;
            MapMarkerRec step;
            step.key = key;
            step.parent = parent;
            step.x = mx;
            step.y = my;
            step.icon = 1;
            step.flags = mask;
            step.wpid = id;
            step.wpnum = seq;
            map_marker_waypoints.push_back(step);
        }
    }
}

void map_marker_trim() {
    int spawns = static_cast<int>(map_marker_spawns.size());
    int ways = static_cast<int>(map_marker_waypoints.size());
    map_marker_total = spawns + ways;
    if (map_marker_limit < 0 || map_marker_total <= map_marker_limit) {
        return;
    }
    if (spawns > map_marker_limit) {
        map_marker_waypoints.clear();
        map_marker_spawns.resize(static_cast<size_t>(map_marker_limit));
        return;
    }
    map_marker_waypoints.resize(static_cast<size_t>(map_marker_limit - spawns));
}

int native_calculate_map_marker(void* state) {
    int flags = 0;
    if (g_is_num && g_to_num && g_is_num(state, 1)) {
        flags = map_marker_int(g_to_num(state, 1));
    }
    map_marker_load_objs();
    map_marker_spawns.clear();
    map_marker_waypoints.clear();
    map_marker_total = 0;
    if (map_marker_objs.empty()) {
        return 0;
    }
    std::map<uint32_t, uint8_t> been;
    std::map<uint32_t, uint8_t> cells;
    for (uint32_t tracked : track_obj_ids) {
        map_marker_place(tracked, 0, flags, 0, been, cells);
    }
    map_marker_trim();
    return 0;
}

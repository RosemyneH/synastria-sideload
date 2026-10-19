int32_t map_i32(const uint8_t* row, int offset) {
    int32_t value = 0;
    memcpy(&value, row + offset, 4);
    return value;
}

float map_f32(const uint8_t* row, int offset) {
    float value = 0;
    memcpy(&value, row + offset, 4);
    return value;
}

const uint8_t* map_table(const char* symbol) {
    void* found = nullptr;
    if (!syn_resolve(symbol, &found)) {
        return nullptr;
    }
    return static_cast<const uint8_t*>(found);
}

int32_t* map_global(const char* symbol) {
    void* found = nullptr;
    if (!syn_resolve(symbol, &found)) {
        return nullptr;
    }
    return static_cast<int32_t*>(found);
}

const uint8_t* map_records(const uint8_t* table) {
    const uint8_t* rows = nullptr;
    memcpy(&rows, table + 0x28, sizeof(rows));
    return rows;
}

// ʕ •ᴥ•ʔ✿ WowClientDB by-id slot is +0x30; min +0x14, max +0x10 ✿ ʕ •ᴥ•ʔ
const uint8_t* map_by_id(const uint8_t* table, int32_t id) {
    if (!table) {
        return nullptr;
    }
    int32_t min_id = map_i32(table, 0x14);
    int32_t max_id = map_i32(table, 0x10);
    if (id < min_id || id > max_id) {
        return nullptr;
    }
    const uint8_t** rows = nullptr;
    memcpy(&rows, table + 0x30, sizeof(rows));
    if (!rows) {
        return nullptr;
    }
    return rows[id - min_id];
}

struct MapShift {
    int32_t map_id;
    float x;
    float y;
    int32_t dungeon;
};

MapShift map_shift(const uint8_t* table, int32_t map_id, float x, float y) {
    MapShift out{map_id, x, y, 0};
    if (!table) {
        return out;
    }
    int32_t count = map_i32(table, 0x0c);
    const uint8_t* rows = map_records(table);
    if (!rows || count <= 0) {
        return out;
    }
    for (int32_t i = 0; i < count; ++i) {
        const uint8_t* rec = rows + static_cast<size_t>(i) * 0x28;
        if (map_i32(rec, 4) != map_id) {
            continue;
        }
        if (map_f32(rec, 0x08) > x || x > map_f32(rec, 0x10) || y < map_f32(rec, 0x0c) || y > map_f32(rec, 0x14)) {
            continue;
        }
        out.map_id = map_i32(rec, 0x18);
        out.x = x + map_f32(rec, 0x1c);
        out.y = y + map_f32(rec, 0x20);
        out.dungeon = map_i32(rec, 0x24);
        return out;
    }
    return out;
}

const uint8_t* map_continent_rec(const uint8_t* table, int32_t map_id) {
    if (!table) {
        return nullptr;
    }
    int32_t count = map_i32(table, 0x0c);
    const uint8_t* rows = map_records(table);
    if (!rows || count <= 0) {
        return nullptr;
    }
    for (int32_t i = count - 1; i >= 0; --i) {
        const uint8_t* rec = rows + static_cast<size_t>(i) * 0x38;
        if (map_i32(rec, 4) == map_id) {
            return rec;
        }
    }
    return nullptr;
}

int32_t map_dungeon_chunk(const uint8_t* table, int32_t map_id, int32_t group, float z) {
    if (!table || map_id <= 0 || group <= 0) {
        return -1;
    }
    int32_t count = map_i32(table, 0x0c);
    const uint8_t* rows = map_records(table);
    if (!rows || count <= 0) {
        return -1;
    }
    bool found_map = false;
    bool found_group = false;
    int32_t dungeon = -1;
    for (int32_t i = 0; i < count; ++i) {
        const uint8_t* rec = rows + static_cast<size_t>(i) * 0x14;
        if (map_i32(rec, 4) != map_id) {
            if (found_map) {
                return dungeon;
            }
            continue;
        }
        found_map = true;
        if (map_i32(rec, 8) == group) {
            found_group = true;
            if (z >= map_f32(rec, 0x10)) {
                dungeon = map_i32(rec, 0x0c);
            }
        } else if (found_group) {
            return dungeon;
        }
    }
    return dungeon;
}

float map_norm(float offset, float range) {
    if (offset == 0.f || range == 0.f) {
        return 0.f;
    }
    return static_cast<float>(1.0 - static_cast<double>(offset) / static_cast<double>(range));
}

struct MapWorld {
    const uint8_t* transforms = nullptr;
    const uint8_t* continents = nullptr;
    const uint8_t* areas = nullptr;
    const uint8_t* dungeons = nullptr;
    const uint8_t* chunks = nullptr;
    const uint8_t* continent_list = nullptr;
    int32_t* current_continent = nullptr;
    int32_t* current_zone = nullptr;
    int32_t* current_dungeon = nullptr;
    int32_t* orphan = nullptr;
    int32_t* wmo_group = nullptr;
    uint32_t (*map_id)() = nullptr;
    bool ready = false;
};

MapWorld& map_world() {
    static MapWorld world;
    if (!world.ready) {
        world.ready = true;
        world.transforms = map_table("g_worldMapTransformsDB");
        world.continents = map_table("g_worldMapContinentDB");
        world.areas = map_table("g_worldMapAreaDB");
        world.dungeons = map_table("g_dungeonMapDB");
        world.chunks = map_table("g_dungeonMapChunkDB");
        world.continent_list = map_table("_ZN10CGWorldMap12m_continentsE");
        world.current_continent = map_global("_ZN10CGWorldMap18m_currentContinentE");
        world.current_zone = map_global("_ZN10CGWorldMap13m_currentZoneE");
        world.current_dungeon = map_global("_ZN10CGWorldMap19m_currentDungeonMapE");
        world.orphan = map_global("_ZN10CGWorldMap19m_currentOrphanZoneE");
        world.wmo_group = map_global("_ZN8CGGameUI12m_wmoGroupIDE");
        void* found = nullptr;
        if (syn_resolve("_Z18ClntObjMgrGetMapIDv", &found)) {
            world.map_id = reinterpret_cast<uint32_t (*)()>(found);
        }
    }
    return world;
}

int32_t map_round_id(double value) {
    if (!(value > -2147483648.0)) {
        return static_cast<int32_t>(0x80000000u);
    }
    if (!(value < 2147483647.0)) {
        return 0x7fffffff;
    }
    double rounded = __builtin_round(value);
    if (!(rounded > -2147483648.0)) {
        return static_cast<int32_t>(0x80000000u);
    }
    if (!(rounded < 2147483647.0)) {
        return 0x7fffffff;
    }
    return static_cast<int32_t>(rounded);
}

bool map_on_world(const MapWorld& world, float world_x, float world_y, float world_z, int32_t map_id, float* out_x, float* out_y) {
    *out_x = 0.f;
    *out_y = 0.f;
    if (!world.current_continent || !world.current_zone || !world.current_dungeon || !world.orphan || !world.wmo_group || !world.continent_list) {
        return false;
    }
    MapShift shift = map_shift(world.transforms, map_id, world_x, world_y);
    if (map_continent_rec(world.continents, shift.map_id) && *world.current_continent == -1) {
        return false;
    }

    int32_t area_id = 0;
    int32_t continent = *world.current_continent;
    if (continent < 0) {
        area_id = *world.orphan;
    } else {
        uint32_t count = 0;
        memcpy(&count, world.continent_list + 4, 4);
        if (static_cast<uint32_t>(continent) >= count) {
            return false;
        }
        const uint8_t* data = nullptr;
        memcpy(&data, world.continent_list + 8, sizeof(data));
        if (!data) {
            return false;
        }
        const uint8_t* entry = data + static_cast<size_t>(continent) * 0x10030;
        if (shift.map_id != map_i32(entry, 0)) {
            return false;
        }
        if (*world.current_zone < 0) {
            area_id = map_i32(entry, 4);
        } else {
            const int32_t* zones = nullptr;
            memcpy(&zones, entry + 0x10, sizeof(zones));
            if (!zones) {
                return false;
            }
            area_id = zones[*world.current_zone];
        }
    }
    if (area_id == -1) {
        return false;
    }
    const uint8_t* area = map_by_id(world.areas, area_id);
    if (!area) {
        return false;
    }
    int32_t area_map = map_i32(area, 4);
    if (area_map != map_id && area_map != shift.map_id) {
        return false;
    }

    float pos_x = *world.current_zone >= 0 ? world_x : shift.x;
    float pos_y = *world.current_zone >= 0 ? world_y : shift.y;
    int32_t shown = *world.orphan == -1 ? -1 : *world.current_dungeon;
    int32_t chunk = map_dungeon_chunk(world.chunks, map_id, *world.wmo_group, world_z);
    const uint8_t* dungeon = nullptr;
    if (chunk == shown) {
        dungeon = map_by_id(world.dungeons, chunk);
    } else if (shift.dungeon == shown) {
        dungeon = map_by_id(world.dungeons, shift.dungeon);
    } else if (shown > 0) {
        return false;
    }

    float max_x = dungeon ? map_f32(dungeon, 0x10) : map_f32(area, 0x18);
    float min_x = dungeon ? map_f32(dungeon, 0x0c) : map_f32(area, 0x1c);
    float max_y = dungeon ? map_f32(dungeon, 0x18) : map_f32(area, 0x20);
    float min_y = dungeon ? map_f32(dungeon, 0x14) : map_f32(area, 0x24);
    *out_x = map_norm(pos_y - min_x, max_x - min_x);
    *out_y = map_norm(pos_x - min_y, max_y - min_y);
    if (*out_x < 0.f || *out_x > 1.f || *out_y < 0.f || *out_y > 1.f) {
        *out_x = 0.f;
        *out_y = 0.f;
    }
    return true;
}

// ʕ •ᴥ•ʔ✿ x, y, optional z, optional map. Pushes normalized map x, y, or nothing when the point is off the open map. ✿ ʕ •ᴥ•ʔ
int native_translate_map_position(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    MapWorld& world = map_world();
    if (!world.map_id) {
        return 0;
    }
    int32_t map_id = static_cast<int32_t>(world.map_id());
    float x = static_cast<float>(g_to_num(state, 1));
    float y = static_cast<float>(g_to_num(state, 2));
    float z = 0.f;
    if (g_is_num(state, 3)) {
        z = static_cast<float>(g_to_num(state, 3));
    }
    if (g_is_num(state, 4)) {
        map_id = map_round_id(g_to_num(state, 4));
    }
    float out_x = 0.f;
    float out_y = 0.f;
    if (!map_on_world(world, x, y, z, map_id, &out_x, &out_y)) {
        return 0;
    }
    g_push_num(state, static_cast<double>(out_x));
    g_push_num(state, static_cast<double>(out_y));
    return 2;
}

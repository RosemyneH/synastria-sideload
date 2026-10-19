int32_t* zone_our_int(const char* symbol) {
    void* found = nullptr;
    if (!syn_resolve(symbol, &found)) {
        return nullptr;
    }
    return static_cast<int32_t*>(found);
}

const uint8_t* zone_our_db(const char* symbol) {
    void* found = nullptr;
    if (!syn_resolve(symbol, &found)) {
        return nullptr;
    }
    return static_cast<const uint8_t*>(found);
}

int zone_our_to_int(double n) {
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

const uint8_t* zone_our_record(const uint8_t* table, int id) {
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

int zone_our_climb(const uint8_t* table, int id, int depth) {
    int32_t min_id = 0;
    memcpy(&min_id, table + 0x14, 4);
    if (id < min_id) {
        return 0;
    }
    for (;;) {
        const uint8_t* row = zone_our_record(table, id);
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

int zone_our_area_root(const uint8_t* table, int id) {
    const uint8_t* row = zone_our_record(table, id);
    if (!row) {
        return 0;
    }
    int32_t parent = 0;
    memcpy(&parent, row + 8, 4);
    if (parent == 0) {
        return id;
    }
    return zone_our_climb(table, parent, 1);
}

bool zone_our_listed(const int* ids, int count, int map) {
    for (int i = 0; i < count; ++i) {
        if (ids[i] == map) {
            return true;
        }
    }
    return false;
}

int zone_our_kind(int map) {
    static const int heroic[] = {
        269, 540, 542, 543, 545, 546, 547, 552, 553, 554, 555, 556, 557, 558, 560, 574, 575, 576,
        578, 585, 595, 599, 600, 601, 602, 604, 608, 619, 632, 650, 658, 668,
    };
    static const int sized[] = {249, 533, 603, 615, 616, 624};
    static const int flex[] = {631, 649, 724};
    if (zone_our_listed(heroic, 32, map)) {
        return 0;
    }
    if (zone_our_listed(sized, 6, map)) {
        return 1;
    }
    if (zone_our_listed(flex, 3, map)) {
        return 2;
    }
    return 3;
}

uint16_t zone_our_bits(uint16_t base, int map, int diff) {
    if (diff == 10) {
        return static_cast<uint16_t>(base | 0x2400);
    }
    int kind = zone_our_kind(map);
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

// ʕ •ᴥ•ʔ✿ instance map id with bit15, difficulty from game data 12/11 unless the arg is > 0; else AreaTable root of subzone, then area ✿ ʕ •ᴥ•ʔ
int native_get_current_zone_our(void* state) {
    static int32_t* map_id = nullptr;
    static int32_t* subzone = nullptr;
    static int32_t* area = nullptr;
    static const uint8_t* maps = nullptr;
    static const uint8_t* areas = nullptr;
    static bool bound = false;
    if (!bound) {
        bound = true;
        map_id = zone_our_int("_ZN8CGGameUI15m_iCurrentMapIDE");
        subzone = zone_our_int("_ZN8CGGameUI11m_subzoneIDE");
        area = zone_our_int("_ZN8CGGameUI8m_areaIDE");
        maps = zone_our_db("g_mapDB");
        areas = zone_our_db("g_areaTableDB");
    }
    bool positive = false;
    if (g_is_num && g_to_num && g_is_num(state, 1)) {
        positive = zone_our_to_int(g_to_num(state, 1)) > 0;
    }
    if (map_id && maps) {
        int map = *map_id;
        bool continent = map == 0 || map == 1 || map == 530 || map == 571;
        if (!continent && zone_our_record(maps, map)) {
            uint16_t base = static_cast<uint16_t>(static_cast<uint32_t>(map) | 0x8000u);
            if (!positive) {
                base = zone_our_bits(base, map, static_cast<int>(game_num(12, 11)));
            }
            if (base) {
                g_push_num(state, base);
                return 1;
            }
        }
    }
    if (!subzone || !area || !areas) {
        return 0;
    }
    const int ids[2] = {*subzone, *area};
    for (int id : ids) {
        int root = zone_our_area_root(areas, id);
        if (root <= 0) {
            continue;
        }
        uint16_t zone = static_cast<uint16_t>(root);
        if (!zone) {
            return 0;
        }
        g_push_num(state, zone);
        return 1;
    }
    return 0;
}

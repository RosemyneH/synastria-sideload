int32_t* zone_client_int(const char* symbol) {
    void* found = nullptr;
    if (!syn_resolve(symbol, &found)) {
        return nullptr;
    }
    return static_cast<int32_t*>(found);
}

bool zone_area_parent(const uint8_t* table, int id, int32_t* parent) {
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
    const uint8_t* row = static_cast<const uint8_t*>(by_id[id - min_id]);
    if (!row) {
        return false;
    }
    memcpy(parent, row + 8, 4);
    return true;
}

int zone_root(const uint8_t* table, int id) {
    int32_t parent = 0;
    if (!zone_area_parent(table, id, &parent)) {
        return 0;
    }
    if (parent == 0) {
        return id;
    }
    id = parent;
    for (int depth = 1;; ++depth) {
        if (!zone_area_parent(table, id, &parent)) {
            return 0;
        }
        if (parent == 0) {
            return id;
        }
        if (depth >= 10) {
            return 0;
        }
        id = parent;
    }
}

int native_get_current_zone(void* state) {
    static int32_t* subzone = nullptr;
    static int32_t* area = nullptr;
    static const uint8_t* table = nullptr;
    static bool bound = false;
    if (!bound) {
        bound = true;
        subzone = zone_client_int("_ZN8CGGameUI11m_subzoneIDE");
        area = zone_client_int("_ZN8CGGameUI8m_areaIDE");
        void* found = nullptr;
        if (syn_resolve("g_areaTableDB", &found)) {
            table = static_cast<const uint8_t*>(found);
        }
    }
    if (!subzone || !area || !table) {
        return 0;
    }
    // ʕ •ᴥ•ʔ✿ subzone first, then area; climb AreaTable.m_parentAreaID to a root ✿ ʕ •ᴥ•ʔ
    const int ids[2] = {*subzone, *area};
    for (int id : ids) {
        int zone = zone_root(table, id);
        if (zone > 0) {
            g_push_num(state, zone);
            return 1;
        }
    }
    return 0;
}

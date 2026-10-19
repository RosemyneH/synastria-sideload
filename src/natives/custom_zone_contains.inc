const uint8_t* zone_contains_table = nullptr;
bool zone_contains_bound = false;

int zone_contains_to_int(double n) {
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

bool zone_contains_continent(int area, int32_t* continent) {
    if (!zone_contains_bound) {
        zone_contains_bound = true;
        void* found = nullptr;
        if (syn_resolve("g_areaTableDB", &found)) {
            zone_contains_table = static_cast<const uint8_t*>(found);
        }
    }
    if (!zone_contains_table) {
        return false;
    }
    int32_t max_id = 0;
    int32_t min_id = 0;
    memcpy(&max_id, zone_contains_table + 0x10, 4);
    memcpy(&min_id, zone_contains_table + 0x14, 4);
    if (area < min_id || area > max_id) {
        return false;
    }
    void** by_id = nullptr;
    memcpy(&by_id, zone_contains_table + 0x30, sizeof(by_id));
    if (!by_id) {
        return false;
    }
    const uint8_t* row = static_cast<const uint8_t*>(by_id[area - min_id]);
    if (!row) {
        return false;
    }
    memcpy(continent, row + 4, 4);
    return true;
}

// ʕ •ᴥ•ʔ✿ area ids match on 0x3fff; map ids (bit15) share 0x3ff, nibble 10-13 is difficulty; map vs area reads AreaTable+4 (Wow.exe 0x00AD3144 / 0x00AD3140 / 0x00AD3154) ✿ ʕ •ᴥ•ʔ
bool zone_contains_match(uint32_t left, uint32_t right) {
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
    if (!zone_contains_continent(static_cast<int>(right & 0x3fffu), &continent)) {
        return false;
    }
    if (continent != static_cast<int32_t>(left & 0x3ffu)) {
        return false;
    }
    return ((left >> 10) & 0xfu) == 0;
}

int native_zone_contains(void* state) {
    if (!g_is_num || !g_to_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int left = zone_contains_to_int(g_to_num(state, 1));
    int right = zone_contains_to_int(g_to_num(state, 2));
    if (left < 0 || right < 0 || left > 0xffff || right > 0xffff) {
        return 0;
    }
    if (!zone_contains_match(static_cast<uint32_t>(left), static_cast<uint32_t>(right))) {
        return 0;
    }
    g_push_num(state, 1.0);
    return 1;
}

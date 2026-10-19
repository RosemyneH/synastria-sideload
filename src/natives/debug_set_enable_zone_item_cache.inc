// ʕ •ᴥ•ʔ✿ null means the zone item overlay is off. Launch starts there. A positive argument builds it. ✿ ʕ •ᴥ•ʔ
struct ZoneItemCache {
    float scale = 1.f;
};

ZoneItemCache* zone_item_cache = nullptr;

int zone_cache_round(double value) {
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

void zone_cache_off() {
    delete zone_item_cache;
    zone_item_cache = nullptr;
}

// ʕ •ᴥ•ʔ✿ a positive number turns the cache on. Zero, a missing number, or a non-number turns it off. Already in that state pushes nothing. ✿ ʕ •ᴥ•ʔ
int native_debug_zone_item_cache(void* state) {
    bool enable = false;
    if (g_is_num && g_to_num && g_is_num(state, 1)) {
        enable = zone_cache_round(g_to_num(state, 1)) > 0;
    }
    if (enable) {
        if (zone_item_cache) {
            return 0;
        }
        zone_item_cache = new ZoneItemCache();
    } else {
        if (!zone_item_cache) {
            return 0;
        }
        zone_cache_off();
    }
    if (g_push_num) {
        g_push_num(state, 1.0);
        return 1;
    }
    return 0;
}

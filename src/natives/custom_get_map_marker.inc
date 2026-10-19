int32_t marker_get_index(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return 0x7fffffff;
    }
    if (!(value < k_max)) {
        return 0x7ffffffe;
    }
    double rounded = __builtin_round(value);
    if (!(rounded > k_min)) {
        return 0x7fffffff;
    }
    if (!(rounded < k_max)) {
        return 0x7ffffffe;
    }
    return static_cast<int32_t>(rounded) - 1;
}

// ʕ •ᴥ•ʔ✿ No argument pushes the live spawn plus waypoint count. A 1-based index walks waypoints from the end, then spawns from the end. ✿ ʕ •ᴥ•ʔ
int native_get_map_marker(void* state) {
    int spawns = static_cast<int>(map_marker_spawns.size());
    int ways = static_cast<int>(map_marker_waypoints.size());
    int total = spawns + ways;
    if (!g_push_num) {
        return 0;
    }
    if (g_is_num && g_to_num && g_is_num(state, 1)) {
        int32_t slot = marker_get_index(g_to_num(state, 1));
        if (slot < 0 || slot >= total) {
            return 0;
        }
        int from_end = total - slot - 1;
        const MapMarkerRec& rec = from_end < spawns
            ? map_marker_spawns[static_cast<size_t>(from_end)]
            : map_marker_waypoints[static_cast<size_t>(from_end - spawns)];
        g_push_num(state, static_cast<double>(rec.key));
        g_push_num(state, static_cast<double>(rec.x));
        g_push_num(state, static_cast<double>(rec.y));
        g_push_num(state, static_cast<double>(rec.icon));
        g_push_num(state, static_cast<double>(rec.wpid));
        g_push_num(state, static_cast<double>(rec.wpnum));
        g_push_num(state, static_cast<double>(rec.parent));
        g_push_num(state, static_cast<double>(rec.flags));
        return 8;
    }
    g_push_num(state, static_cast<double>(total));
    return 1;
}

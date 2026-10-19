std::set<uint32_t> track_obj_ids;
std::vector<uint32_t> track_obj_order;

// ʕ •ᴥ•ʔ✿ round half away from 0, then clamp to int32 the way the DLL's tonumber path does ✿ ʕ •ᴥ•ʔ
int track_obj_to_int(double value) {
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

// ʕ •ᴥ•ʔ✿ category 0x26 key 0, truncate toward 0; set bit 1 when it is clear ✿ ʕ •ᴥ•ʔ
void track_obj_touch_flag() {
    double raw = game_num(0x26, 0);
    constexpr double k_lim = 9223372036854775808.0;
    int flags = static_cast<int>(0x80000000u);
    if ((raw > -k_lim) && (raw < k_lim)) {
        flags = static_cast<int>(static_cast<int64_t>(raw));
    }
    if ((flags & 2) == 0) {
        set_num(0x26, 0, static_cast<double>(static_cast<uint32_t>(flags) | 2u));
    }
}

// ʕ •ᴥ•ʔ✿ type is 0..255, id is 1..0xFFFFFF; key is (type<<24)|id; a second add of the same key returns 0 ✿ ʕ •ᴥ•ʔ
int native_add_track_obj_loc(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int type = track_obj_to_int(g_to_num(state, 1));
    int id = track_obj_to_int(g_to_num(state, 2));
    if (static_cast<uint32_t>(type) > 0xffu || id <= 0 || (id & 0xff000000) != 0) {
        return 0;
    }
    uint32_t key = (static_cast<uint32_t>(type) << 24) | static_cast<uint32_t>(id);
    if (!track_obj_ids.insert(key).second) {
        return 0;
    }
    track_obj_order.push_back(key);
    track_obj_touch_flag();
    g_push_num(state, 1.0);
    return 1;
}

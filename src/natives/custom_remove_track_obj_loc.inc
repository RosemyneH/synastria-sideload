// ʕ •ᴥ•ʔ✿ type is 0..255, id is 1..0xFFFFFF; drop (type<<24)|id from the set, then the first order match; a missing key returns 0 ✿ ʕ •ᴥ•ʔ
int native_remove_track_obj_loc(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1) || !g_is_num(state, 2)) {
        return 0;
    }
    int type = track_obj_to_int(g_to_num(state, 1));
    int id = track_obj_to_int(g_to_num(state, 2));
    if (static_cast<uint32_t>(type) > 0xffu || id <= 0 || (id & 0xff000000) != 0) {
        return 0;
    }
    uint32_t key = (static_cast<uint32_t>(type) << 24) | static_cast<uint32_t>(id);
    if (track_obj_ids.erase(key) == 0) {
        return 0;
    }
    auto found = std::find(track_obj_order.begin(), track_obj_order.end(), key);
    if (found != track_obj_order.end()) {
        track_obj_order.erase(found);
    }
    track_obj_touch_flag();
    g_push_num(state, 1.0);
    return 1;
}

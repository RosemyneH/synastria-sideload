// ʕ •ᴥ•ʔ✿ no args; a zero set size returns 0; otherwise drop every key, set category 0x26 bit 1, push 1 ✿ ʕ •ᴥ•ʔ
int native_clear_track_obj_loc(void* state) {
    if (track_obj_ids.empty()) {
        return 0;
    }
    track_obj_ids.clear();
    track_obj_order.clear();
    track_obj_touch_flag();
    g_push_num(state, 1.0);
    return 1;
}

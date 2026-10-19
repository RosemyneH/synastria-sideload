using TrackAllTableFn = void (*)(void*, int, int);
using TrackAllRawFn = void (*)(void*, int);
using TrackAllCheckFn = int (*)(void*, int);

TrackAllTableFn track_all_table = nullptr;
TrackAllRawFn track_all_rawset = nullptr;
TrackAllCheckFn track_all_check = nullptr;
bool track_all_bound = false;

void track_all_bind() {
    if (track_all_bound) {
        return;
    }
    track_all_bound = true;
    void* found = nullptr;
    if (syn_resolve("lua_createtable", &found)) {
        track_all_table = reinterpret_cast<TrackAllTableFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_rawset", &found)) {
        track_all_rawset = reinterpret_cast<TrackAllRawFn>(found);
    }
    found = nullptr;
    if (syn_resolve("lua_checkstack", &found)) {
        track_all_check = reinterpret_cast<TrackAllCheckFn>(found);
    }
}

// ʕ •ᴥ•ʔ✿ no args; empty order returns 0; otherwise a 1-based table of (type<<24)|id doubles, order kept ✿ ʕ •ᴥ•ʔ
int native_get_all_track_obj_loc(void* state) {
    if (track_obj_order.empty()) {
        return 0;
    }
    track_all_bind();
    if (!track_all_table || !track_all_rawset || !g_push_num) {
        return 0;
    }
    track_all_table(state, 0, 0);
    int index = 1;
    for (uint32_t key : track_obj_order) {
        if (track_all_check) {
            track_all_check(state, 2);
        }
        g_push_num(state, static_cast<double>(index));
        g_push_num(state, static_cast<double>(key));
        track_all_rawset(state, -3);
        ++index;
    }
    return 1;
}

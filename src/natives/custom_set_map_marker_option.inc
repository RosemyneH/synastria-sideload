bool marker_opt_int(double value, int& out) {
    constexpr double lo = -2147483648.0;
    constexpr double hi = 2147483647.0;
    if (!(lo < value) || !(value < hi)) {
        return false;
    }
    double rounded = __builtin_round(value);
    if (!(lo < rounded) || !(rounded < hi)) {
        return false;
    }
    out = static_cast<int>(rounded);
    return true;
}

// ʕ •ᴥ•ʔ✿ 1 reads or sets the marker cap when the value is in [0, 99999]. 2 reads or sets cell size in [0, 100]. Anything else pushes 0. ✿ ʕ •ᴥ•ʔ
int native_set_map_marker_option(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num) {
        return 0;
    }
    int option = 0;
    if (!g_is_num(state, 1) || !marker_opt_int(g_to_num(state, 1), option)) {
        g_push_num(state, 0.0);
        return 1;
    }
    if (option == 1) {
        int next = 0;
        if (g_is_num(state, 2) && marker_opt_int(g_to_num(state, 2), next) && next >= 0 && next < 100000) {
            map_marker_limit = next;
        }
        g_push_num(state, static_cast<double>(static_cast<float>(map_marker_limit)));
        return 1;
    }
    if (option == 2) {
        if (g_is_num(state, 2)) {
            float cell = static_cast<float>(g_to_num(state, 2));
            if (cell >= 0.f && cell <= 100.f) {
                map_marker_cell = cell;
            }
        }
        g_push_num(state, static_cast<double>(map_marker_cell));
        return 1;
    }
    g_push_num(state, 0.0);
    return 1;
}

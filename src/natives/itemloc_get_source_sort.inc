// ʕ •ᴥ•ʔ✿ Record +0x0e is the source sort mode. Load stores 3; a zero slot is that default. ✿ ʕ •ᴥ•ʔ
uint8_t src_sort_get_mode[0x10000];

int src_sort_get_item(void* state) {
    if (!g_is_num(state, 1)) {
        return 0;
    }
    double value = g_to_num(state, 1);
    if (!(value > -2147483648.0) || !(value < 2147483647.0)) {
        return 0;
    }
    long long rounded = value >= 0.0
        ? static_cast<long long>(value + 0.5)
        : static_cast<long long>(value - 0.5);
    if (rounded <= (-2147483647LL - 1) || rounded >= 2147483647LL) {
        return 0;
    }
    int id = static_cast<int>(rounded);
    if (id <= 0 || id >= 0x10000) {
        return 0;
    }
    return id;
}

int native_itemloc_get_source_sort(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num) {
        return 0;
    }
    int id = src_sort_get_item(state);
    if (id == 0 || !src_at_ready || src_at_items[static_cast<size_t>(id)].empty()) {
        return 0;
    }
    unsigned mode = src_sort_get_mode[id];
    if (mode == 0) {
        mode = 3;
    }
    g_push_num(state, static_cast<double>(mode));
    return 1;
}

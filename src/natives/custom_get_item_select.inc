int32_t select_get_slot(double value) {
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

// ʕ •ᴥ•ʔ✿ 1-based slot. Slots at 100 and above stay empty even when the filtered list is longer. ✿ ʕ •ᴥ•ʔ
int native_get_item_select(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num || !g_is_num(state, 1)) {
        return 0;
    }
    int32_t index = select_get_slot(g_to_num(state, 1));
    if (index < 0) {
        return 0;
    }
    int32_t count = static_cast<int32_t>(item_select.size());
    if (index >= count || index >= 0x64) {
        return 0;
    }
    g_push_num(state, static_cast<double>(item_select[static_cast<size_t>(index)]->id));
    return 1;
}

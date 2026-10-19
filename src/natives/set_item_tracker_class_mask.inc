// ʕ •ᴥ•ʔ✿ WoWExt dword 0x100aed30 starts at 0. A number replaces it and returns nothing; otherwise that mask is pushed. ✿ ʕ •ᴥ•ʔ
int32_t tracker_mask_value = 0;

int32_t tracker_mask_to_int(double value) {
    constexpr double k_min = -2147483648.0;
    constexpr double k_max = 2147483647.0;
    if (!(value > k_min)) {
        return static_cast<int32_t>(0x80000000u);
    }
    if (!(value < k_max)) {
        return 0x7fffffff;
    }
    double rounded = __builtin_round(value);
    if (!(rounded > k_min)) {
        return static_cast<int32_t>(0x80000000u);
    }
    if (!(rounded < k_max)) {
        return 0x7fffffff;
    }
    return static_cast<int32_t>(rounded);
}

int native_item_tracker_class_mask(void* state) {
    if (!g_is_num || !g_to_num || !g_push_num) {
        return 0;
    }
    if (g_is_num(state, 1)) {
        tracker_mask_value = tracker_mask_to_int(g_to_num(state, 1));
        return 0;
    }
    g_push_num(state, static_cast<double>(tracker_mask_value));
    return 1;
}
